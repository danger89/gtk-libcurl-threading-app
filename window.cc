#include "window.h"
#include <iostream>

/** CURL callback for writing the result to a stream. */
static size_t curl_cb_stream(
    /** [in] Pointer to the result. */
    char *ptr,
    /** [in] Size each chunk of the result. */
    size_t size,
    /** [in] Number of chunks in the result. */
    size_t nmemb,
    /** [out] Response (a pointer to `std::iostream`). */
    void *response_void)
{
  std::iostream *response = static_cast<std::iostream *>(response_void);

  const size_t n = size * nmemb;

  response->write(ptr, n);

  return n;
}

MainWindow::MainWindow() : m_vbox(Gtk::ORIENTATION_VERTICAL),
                           m_button("Start thread"),
                           m_button1("Stop thread"),
                           m_label("Output logging:"),
                           thread_(nullptr),
                           is_thread_done_(false),
                           stop_running_thread_(false),
                           still_running_(0)
{
  set_title("C++ Threading with cURL Example App");
  set_default_size(1000, 680);
  set_position(Gtk::WIN_POS_CENTER);

  m_label.set_xalign(0.0);

  m_button.signal_clicked().connect(sigc::mem_fun(this, &MainWindow::startThread));
  m_button1.signal_clicked().connect(sigc::mem_fun(this, &MainWindow::stopThread));

  // Add all elements to the horizontal box
  m_hbox.pack_start(m_button, false, false, 4);
  m_hbox.pack_start(m_button1, false, false, 4);

  // Add horizontal box and textview to the verticial box
  m_vbox.pack_start(m_hbox, false, false, 4);
  m_vbox.pack_start(m_label, false, false, 4);
  m_vbox.pack_end(m_textview, true, true, 4);

  // Curl global init
  curl_global_init(CURL_GLOBAL_ALL);

  // init a multi stack
  multi_handle_ = curl_multi_init();

  // Create an easy handle
  curl_ = curl_easy_init();

  add(m_vbox);
  show_all_children();
}

/**
 * Destructor
 */
MainWindow::~MainWindow()
{
  curl_multi_remove_handle(multi_handle_, curl_);
  curl_easy_cleanup(curl_);
  curl_global_cleanup();
}

/**
 * Insert text to the end of the textview
 */
void MainWindow::insertLoggingText(const std::string &text)
{
  auto textViewBuffer = m_textview.get_buffer();
  auto endIter = textViewBuffer->end();
  textViewBuffer->insert(endIter, text + "\n");
}

void MainWindow::startThread()
{
  // Stop / join current thread, if needed
  this->stopThread();

  if (thread_ == nullptr)
  {
    this->insertLoggingText("Start thread");
    thread_ = new std::thread(&MainWindow::request, this);
  }
}

void MainWindow::stopThread()
{
  // TODO: ->detatch() iso ->join() is maybe also fine!
  if (thread_ && thread_->joinable())
  {
    if (is_thread_done_)
    {
      this->insertLoggingText("Reset thread");
      thread_->join();
    }
    else
    {
      this->insertLoggingText("Stop thread");
      // Trigger the thread to stop now
      // This is only possible with the cURL multi API
      stop_running_thread_ = true;
      thread_->join();
      stop_running_thread_ = false;
    }
    delete thread_;
    thread_ = nullptr;
    is_thread_done_ = false; // reset
  }
}

/**
 * Running the request inside a thread
 */
void MainWindow::request()
{
  std::stringstream response; // Needs to be stored outside the thread

  // Re-use the single handle, add options
  std::string url = "https://www.google.com/";
  curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, curl_cb_stream);
  curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);
  curl_easy_setopt(curl_, CURLOPT_NOSIGNAL, 1L);

  // easy handle
  curl_multi_add_handle(multi_handle_, curl_);

  do
  {
    CURLMcode mc = curl_multi_perform(multi_handle_, &still_running_);

    if (stop_running_thread_)
      break;

    if (!mc && still_running_)
      /* wait for activity, timeout or "nothing", avoid busy waiting */
      mc = curl_multi_poll(multi_handle_, NULL, 0, 1000, NULL);

    if (mc)
    {
      std::cerr << "curl_multi_poll() failed, code: " << mc << std::endl;
      break;
    }

  } while (still_running_);

  curl_multi_remove_handle(multi_handle_, curl_);

  // Just for info
  std::cout << "Body:" << response.str() << std::endl;

  is_thread_done_ = true;
}
#include "window.h"
#include <iostream>

/** CURL callback for writing the result to a stream. */
static size_t curl_cb_stream(
    /** [in] Pointer to the result. */
    char* ptr,
    /** [in] Size each chunk of the result. */
    size_t size,
    /** [in] Number of chunks in the result. */
    size_t nmemb,
    /** [out] Response (a pointer to `std::iostream`). */
    void* response_void)
{
  std::iostream* response = static_cast<std::iostream*>(response_void);

  const size_t n = size * nmemb;

  response->write(ptr, n);

  return n;
}

MainWindow::MainWindow()
    : m_vbox(Gtk::ORIENTATION_VERTICAL),
      m_button_start("Start thread"),
      m_button_stop("Stop thread"),
      m_button_start_multiple("Start multiple threads"),
      m_button_stop_multiple("Stop multiple threads"),
      m_label("Output logging:"),
      thread_(nullptr),
      thread_2_(nullptr),
      is_thread_done_(false),
      is_thread_multi_req_done_(false),
      keep_thread_running_(true),
      keep_thread_multi_req_running_(true)
{
  set_title("C++ Threading with cURL Example App");
  set_default_size(1000, 680);
  set_position(Gtk::WIN_POS_CENTER);

  m_label.set_xalign(0.0);

  m_button_start.signal_clicked().connect(sigc::mem_fun(this, &MainWindow::startThread));
  m_button_stop.signal_clicked().connect(sigc::mem_fun(this, &MainWindow::stopThread));
  m_button_start_multiple.signal_clicked().connect(sigc::mem_fun(this, &MainWindow::startMultipleRequestThread));
  m_button_stop_multiple.signal_clicked().connect(sigc::mem_fun(this, &MainWindow::stopMultipleRequestsThread));

  // End marker of textview
  m_endMark = m_textview.get_buffer()->create_mark(m_textview.get_buffer()->end(), false);

  m_scrolledWindow.add(m_textview); // scrollable textview

  m_entry.set_text("https://www.google.com"); // Initial URL

  // Add all elements to the horizontal boxes
  m_hbox.pack_start(m_button_start, false, false, 4);
  m_hbox.pack_start(m_button_stop, false, false, 4);
  m_hbox1.pack_start(m_button_start_multiple, false, false, 4);
  m_hbox1.pack_start(m_button_stop_multiple, false, false, 4);

  // Add horizontal box and textview to the verticial box
  m_vbox.pack_start(m_entry, false, true, 4);
  m_vbox.pack_start(m_hbox, false, false, 4);
  m_vbox.pack_start(m_hbox1, false, false, 4);
  m_vbox.pack_start(m_label, false, false, 4);
  m_vbox.pack_end(m_scrolledWindow, true, true, 4);

  // Curl global init
  curl_global_init(CURL_GLOBAL_ALL);

  // init a multi stack, for the single thread example
  multi_handle_ = curl_multi_init();

  // Init a seperate multi stack, for the multiple threads example
  multi_handle_2_ = curl_multi_init();
  // Limit the parallel conenctions to 10
  curl_multi_setopt(multi_handle_2_, CURLMOPT_MAXCONNECTS, 10);

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
  this->stopThread();
  this->stopMultipleRequestsThread();

  curl_multi_remove_handle(multi_handle_, curl_);
  curl_multi_cleanup(multi_handle_);
  curl_multi_cleanup(multi_handle_2_);
  curl_easy_cleanup(curl_);
  curl_global_cleanup();
}

/**
 * Insert text to the end of the textview
 */
void MainWindow::insertLoggingText(const std::string& text)
{
  auto textViewBuffer = m_textview.get_buffer();
  auto endIter = textViewBuffer->end();
  // Add text
  textViewBuffer->insert(endIter, text + "\n");
  // Scroll to end marker
  m_textview.scroll_to(m_endMark);
}

/**
 * Start request in a thread. Always stop the previous!
 */
void MainWindow::startThread()
{
  // Stop / join current thread, if needed
  this->stopThread();

  if (thread_ == nullptr)
  {
    std::string url = m_entry.get_text();
    this->insertLoggingText("Start thread, URL: " + url);
    thread_ = new std::thread(&MainWindow::request, this, url);
  }
}

/**
 * Start multiple requests thread, which does multiple requests in parallel, leveraging the cURL Multi API.
 * A single thread is enough to do parallel requests.
 */
void MainWindow::startMultipleRequestThread()
{
  // Stop / join current thread, if needed
  this->stopMultipleRequestsThread();

  if (thread_2_ == nullptr)
  {
    std::string url = m_entry.get_text();
    this->insertLoggingText("Start multiple requests (10) thread, URL: " + url);
    std::vector<std::string> urls;

    // Add 10 (same) requests in parallel
    for (int i = 0; i < 10; i++)
    {
      urls.push_back(url);
    }

    thread_2_ = new std::thread(&MainWindow::multipleRequest, this, urls);
  }
}

/**
 * Stop the running thread, if applicable.
 */
void MainWindow::stopThread()
{
  if (thread_ && thread_->joinable())
  {
    if (is_thread_done_)
    {
      this->insertLoggingText("Only reset thread");
      thread_->join();
    }
    else
    {
      this->insertLoggingText("Stop thread");
      // Trigger the thread to stop now
      // This is only possible with the cURL multi API
      keep_thread_running_ = false;
      thread_->join();
      keep_thread_running_ = true;
    }
    delete thread_;
    thread_ = nullptr;
    is_thread_done_ = false; // reset
  }
}

/**
 * Stop the multiple requests thread
 */
void MainWindow::stopMultipleRequestsThread()
{
  if (thread_2_ && thread_2_->joinable())
  {
    if (is_thread_multi_req_done_)
    {
      this->insertLoggingText("Only reset multi request thread");
      thread_2_->join();
    }
    else
    {
      this->insertLoggingText("Stop multi request thread");
      // Trigger the thread to stop now
      // This is only possible with the cURL multi API
      keep_thread_multi_req_running_ = false;
      thread_2_->join();
      keep_thread_multi_req_running_ = true;
    }
    delete thread_2_;
    thread_2_ = nullptr;
    is_thread_multi_req_done_ = false; // reset
  }
}

/**
 * Running the request inside a thread
 */
void MainWindow::request(const std::string& url)
{
  int still_running = 0;
  std::stringstream response;

  // We need to be able to pass 'keep_thread_running_' boolean through the IPFS C++ client interface.
  // Which can then be used internally, inside the C++ IPFS client, in the while loop as shown below.
  /* IPFS C++ curl lib interface can start here */

  // Re-use the single handle, add options
  curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, curl_cb_stream);
  curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);
  curl_easy_setopt(curl_, CURLOPT_NOSIGNAL, 1L);

  // Add easy handle
  curl_multi_add_handle(multi_handle_, curl_);

  do
  {
    CURLMcode mc = curl_multi_perform(multi_handle_, &still_running);

    // Allow to break/stop the thread at any given moment
    if (!keep_thread_running_)
      break;

    if (!mc && still_running)
      /* wait for activity, timeout or "nothing" */
      mc = curl_multi_poll(multi_handle_, NULL, 0, 40, NULL);

    if (mc)
    {
      std::cerr << "curl_multi_poll() failed, code: " << mc << std::endl;
      break;
    }

  } while (still_running);

  curl_multi_remove_handle(multi_handle_, curl_);

  /* IPFS C++ curl lib interface stops here */

  // Only process response when we did not stop,
  // otherwise the main thread will be blocked on str()
  if (keep_thread_running_)
  {
    // Output response just for info
    std::cout << "Body:" << response.str() << std::endl;
  }

  is_thread_done_ = true;
}

/**
 * Supporting multiple requests inside a thread
 */
void MainWindow::multipleRequest(std::vector<std::string> urls)
{
  int still_running = 0;
  CURLMsg* msg;
  int msgs_left = -1;
  std::stringstream response;
  std::vector<CURL*> handles;

  // Create an easy handle for each URL
  for (auto& url : urls)
  {
    CURL* curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_cb_stream);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_multi_add_handle(multi_handle_2_, curl);
    handles.push_back(curl);
  }

  do
  {
    CURLMcode mc = curl_multi_perform(multi_handle_2_, &still_running);

    // Allow to break/stop the thread at any given moment
    if (!keep_thread_multi_req_running_)
      break;

    while ((msg = curl_multi_info_read(multi_handle_2_, &msgs_left)))
    {
      if (msg->msg == CURLMSG_DONE)
      {
        char* url;
        CURL* curl = msg->easy_handle;
        curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &url);
        if (msg->data.result != 0)
          std::cout << "Error code: " << msg->data.result << " Message: " << curl_easy_strerror(msg->data.result)
                    << ". URL: " << url << std::endl;
      }
      else
      {
        std::cout << "Error message: " << msg->msg << std::endl;
      }
    }

    if (!mc && still_running)
      /* wait for activity, timeout or "nothing" */
      mc = curl_multi_poll(multi_handle_2_, NULL, 0, 40, NULL);

    if (mc)
    {
      std::cerr << "curl_multi_poll() failed, code: " << mc << std::endl;
      break;
    }
  } while (still_running);

  // Clean-up
  for (auto& curl : handles)
  {
    curl_multi_remove_handle(multi_handle_2_, curl);
    curl_easy_cleanup(curl);
  }

  // Output of all 10 requests together..
  if (keep_thread_multi_req_running_)
  {
    std::cout << "Body:" << response.str() << std::endl;
  }
}
#include "window.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

MainWindow::MainWindow() : m_vbox(Gtk::ORIENTATION_VERTICAL),
                           m_button("Start thread"),
                           m_button1("Stop thread"),
                           m_label("Output logging:"),
                           stopRunningThread(false)
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

  add(m_vbox);
  show_all_children();
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
  if (thread == nullptr) {
    this->insertLoggingText("Start thread");
    thread = new std::thread(&MainWindow::process, this);
  }
}

void MainWindow::stopThread()
{
  if (thread) {
    if (thread->joinable()) {
      this->insertLoggingText("Stop thread");
      stopRunningThread = true;
      thread->join();
      stopRunningThread = false;
      delete thread;
      thread = nullptr;
    }
  }
}

/**
 * Running process() inside a thread
 */
void MainWindow::process()
{
  while(true) {
    if (stopRunningThread)
      return;
    sleep(0.1);
  }
}
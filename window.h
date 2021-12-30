#include <gtkmm.h>
#include <thread>  
#include <atomic>
#include <curl/curl.h>

class MainWindow : public Gtk::Window
{
public:
  MainWindow();
  ~MainWindow();
protected:
  void startThread();
  void stopThread();

  Gtk::Box m_vbox;
  Gtk::Box m_hbox;
  Gtk::Button m_button;
  Gtk::Button m_button1;
  Gtk::Label  m_label;
  Gtk::TextView m_textview;

private:
  void insertLoggingText(const std::string &text);
  void request();

  std::thread *thread_;
  std::atomic<bool> is_thread_done_;
  std::atomic<bool> stop_running_thread_;
  CURL *curl_;
  CURLM *multi_handle_;
  int still_running_;
};

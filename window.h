#include <atomic>
#include <curl/curl.h>
#include <gtkmm.h>
#include <thread>

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
  Gtk::Entry m_entry;
  Gtk::Label m_label;
  Gtk::TextView m_textview;
  Gtk::ScrolledWindow m_scrolledWindow;
  Glib::RefPtr<Gtk::TextBuffer::Mark> m_endMark;

private:
  void insertLoggingText(const std::string& text);
  void request(const std::string& url);

  std::thread* thread_;
  std::atomic<bool> is_thread_done_;
  std::atomic<bool> keep_thread_running_;
  CURL* curl_;
  CURLM* multi_handle_;
};

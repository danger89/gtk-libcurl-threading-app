#include <atomic>
#include <curl/curl.h>
#include <gtkmm.h>
#include <thread>
#include <vector>

class MainWindow : public Gtk::Window
{
public:
  MainWindow();
  ~MainWindow();

protected:
  void startThread();
  void startMultipleRequestThread();
  void stopThread();
  void stopMultipleRequestsThread();

  Gtk::Box m_vbox;
  Gtk::Box m_hbox;
  Gtk::Box m_hbox1;
  Gtk::Button m_button_start;
  Gtk::Button m_button_stop;
  Gtk::Button m_button_start_multiple;
  Gtk::Button m_button_stop_multiple;
  Gtk::Entry m_entry;
  Gtk::Label m_label;
  Gtk::TextView m_textview;
  Gtk::ScrolledWindow m_scrolledWindow;
  Glib::RefPtr<Gtk::TextBuffer::Mark> m_endMark;

private:
  void insertLoggingText(const std::string& text);
  void request(const std::string& url);
  void multipleRequest(std::vector<std::string> urls);

  std::thread* thread_;                             /* thread for the single request example */
  std::thread* thread_2_;                           /* thread for the multi request example */
  std::atomic<bool> is_thread_done_;                /* indication when the single request is done */
  std::atomic<bool> is_thread_multi_req_done_;      /* indication when the multple request thread is done */
  std::atomic<bool> keep_thread_running_;           /* trigger the thread to stop/continue */
  std::atomic<bool> keep_thread_multi_req_running_; /* trigger the thread with multiple requests to stop/continue */
  CURL* curl_;                                      /* cURL single easy handle for the single request */
  CURLM* multi_handle_;                             /* cURL multiple for the single request example */
  CURLM* multi_handle_2_;                           /* cURL multiple for the multiple requests in parallel example */
};

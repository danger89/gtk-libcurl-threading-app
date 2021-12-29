#include <gtkmm.h>
#include <thread>  
#include <atomic>

class MainWindow : public Gtk::Window
{
public:
  MainWindow();

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
  void process();

  std::thread *thread;
  std::atomic<bool> stopRunningThread;
};

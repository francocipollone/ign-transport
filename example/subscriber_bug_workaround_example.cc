
/*
 * This example tries to show a workaround for the bug presented in `subscriber_bug_example`.
 * The idea is to guarantee the that the instance of the class that holds the cb method is alive until
 * the end of the execution.
 *
 * Execute this after that the publisher is started: `publisher_bug_example`.
*/

#include <iostream>
#include <memory>
#include <string>
#include <ignition/msgs.hh>
#include <ignition/transport.hh>

// Subscribes to a topic and prints the value of the message when the callback method is called.
class Monitor {
 public:
  // Creates an Monitor.
  // @param topic_name Valid ignition transport topic name.
  // @returns An instance of Monitor wrapped by std::shared_ptr.
  friend std::shared_ptr<Monitor> MakeSharedMonitor(const std::string& topic_name);

  ~Monitor() {std::cout << "****\n[Monitor] Class Destructor!\n****\n" << std::endl;}

 private:

  // Constructs a Monitor object.
  // @param topic_name Valid ignition transport topic name.
  explicit Monitor(const std::string& topic_name) : topic_name_(topic_name) {}

  // Subscribes to the ignition transport topic.
  // @param A pointer to this instance.
  void Initialize(std::shared_ptr<Monitor> self_ptr) {
    // A std::shared_ptr to `this` is passed to a instance variable
    // in order to guarantee that the Monitor instance hasn't been destroyed at
    // the moment that the callback method is executed.
    self_ptr_ = self_ptr;
    node_.Subscribe(topic_name_, &Monitor::OnTopicMessage, this);
  }

  // Ignition subscriber callback, updating state.
  //
  // @param message Message received.
  void OnTopicMessage(const ignition::msgs::StringMsg& message) {
    std::cout << "[Callback]" << std::endl;
    std::lock_guard<std::mutex> guard(mutex_);
    last_message_ = message;
    message_count_++;
    std::cout << "Msg: " << message.data() << std::endl << std::endl;

  }

  // Topic name.
  const std::string topic_name_;
  // Holds a pointer to this instance.
  // This will cause that the instance of the class won't be deallocated by the SO
  // until the execution of the unit is finished.
  std::shared_ptr<Monitor> self_ptr_{nullptr};
  // Received message count.
  int message_count_{0};
  // Last ignition message received.
  ignition::msgs::StringMsg last_message_{};
  // Mutex to synchronize state read/write operations.
  mutable std::mutex mutex_{};
  // Ignition transport node for subscription.
  ignition::transport::Node node_{};
};

std::shared_ptr<Monitor> MakeSharedMonitor(const std::string& topic_name){
  auto monitor = std::shared_ptr<Monitor>(new Monitor{topic_name});
  monitor->Initialize(monitor);
  return monitor;
}

//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  const std::string topic = "/foo";

  // During every loop a Monitor object is created by passing the `topic` to be subscribed to.
  // After this, the thread is slept for a certain time, allowing to the callback method to be called several times.
  //
  // When each iteration finishes, the `monitor` object isn't destroyed because it holds a shared_ptr to itself.
  // This guarantee that the callback methods won't show an undefined behavior.
  unsigned int iterations{1};
  for (int i = 1 ; i <= 20 ; ++i ){
    std::cout << "Iteration number: "<< i << std::endl;
    auto monitor = MakeSharedMonitor(topic);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
  return 0;
}

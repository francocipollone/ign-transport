
/*
 * This example tries to show the presence of a bug in which a callback method
 * is executed after that the instance of the class that holds it is destroyed.
 *
 *  Execute this after that the publisher is started: `publisher_bug_example`.
*/

#include <iostream>
#include <string>
#include <ignition/msgs.hh>
#include <ignition/transport.hh>

// Subscribes to a topic and prints the value of the message when the callback method is called.
class Monitor {
 public:
  // Constructs a monitor for the given topic.
  // @param topic_name Valid ignition transport topic name.
  explicit Monitor(const std::string& topic_name) { node_.Subscribe(topic_name, &Monitor::OnTopicMessage, this); }

  ~Monitor() {std::cout << "****\n[Monitor] Class Destructor!\n****\n" << std::endl;}

 private:
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

  // Received message count.
  int message_count_{0};
  // Last ignition message received.
  ignition::msgs::StringMsg last_message_{};
  // Mutex to synchronize state read/write operations.
  mutable std::mutex mutex_{};
  // Ignition transport node for subscription.
  ignition::transport::Node node_{};
};

//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  const std::string topic = "/foo";

  // During every loop a Monitor object is created by passing the `topic` to be subscribed to.
  // After this, the thread is slept for a certain time, allowing to the callback method to be called several times.
  //
  // When each iteration finishes, the `ign_monitor` object is destroyed, expecting that the callback method won't be called after
  // that, given that it would lead to an undefined behaviour(most likely seg fault).
  unsigned int iterations{1};
  while(true){
    std::cout << "Iteration number: "<< iterations << std::endl;
    Monitor ign_monitor{topic};
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    iterations++;
  }
  return 0;
}

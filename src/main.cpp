#include <boost/asio.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>
#include <vector>
#include <atomic>
#include <boost/program_options.hpp>

using boost::asio::ip::tcp;
using namespace std;
namespace po = boost::program_options;

class Client {
public:

    static std::atomic<int> total_sent_counter;
    static std::atomic<int> total_received_counter;

    Client(boost::asio::io_context& io_context, const string& host, const string& port, int id, int response_timeout, bool logging_enabled)
        : io_context_(io_context), socket_(io_context), host_(host), port_(port), id_(id), response_timeout_(response_timeout), logging_enabled_(logging_enabled) {
        connect();
    }

    ~Client() {
        log("closing the socket");
        socket_.close(); // d'tor
    }

    void async_send(const string& request) {
        if (!socket_.is_open()) {
            log("Socket not open, attempting to reconnect...");
            connect(); // attempt reconnect if socket is not open
        }

        // Async write
        boost::asio::async_write(socket_, boost::asio::buffer(request),
            [this](boost::system::error_code ec, size_t) {
                if (!ec) {
                    log("message sent ok");
                    total_sent_counter++;
                    async_readResponse(); 
                } else {
                    stringstream ss;
                    ss << "Send failed: " << ec.message();
                    log(ss.str());
                }
            });
    }

private:
    string host_, port_;
    boost::asio::io_context& io_context_;
    tcp::socket socket_;
    int response_timeout_;
    int id_; // simple id for this connection
    bool logging_enabled_;

    void connect() {
        tcp::resolver resolver(io_context_);
        auto endpoints = resolver.resolve(host_, port_);
        boost::asio::async_connect(socket_, endpoints,
            [this](boost::system::error_code ec, tcp::endpoint) {
                if (!ec) {
                    stringstream ss;
                    ss << "Connected to the server at " << host_ << ":" << port_ << ", ";
                    log(ss.str());
                } else {
                    stringstream ss;
                    ss << "Connection failed: " << ec.message();
                    log(ss.str());
                }
            });
    }

    void async_readResponse() {
        auto receive_buffer = std::make_shared<boost::asio::streambuf>();
        auto timer = std::make_shared<boost::asio::steady_timer>(socket_.get_executor(), std::chrono::seconds(response_timeout_));

        boost::asio::async_read_until(socket_, *receive_buffer, "\r\n",
            [this, receive_buffer, timer](boost::system::error_code ec, std::size_t) {
                timer->cancel(); // cancel timer if hsot responded back
                if (!ec) {
                    std::istream response_stream(receive_buffer.get());
                    std::string response_line;
                    std::getline(response_stream, response_line);
                    stringstream ss;
                    ss << "Response received: " << response_line;
                    total_received_counter++;
                    log(ss.str());
                } else {
                    stringstream ss;
                    ss << "No response: " << ec.message();
                    log(ss.str());
                }
            });

            // start timer to cancel the read operation if too long
            timer->async_wait([this, timer](const boost::system::error_code& ec) {
                if (!ec) {  // check if timer not already cancelled
                    socket_.cancel();  // this will cause the read operation to end with an error
                    log("Read operation timed out.");
                }
            });
    }

    void log(const std::string& message) const {
        if(!logging_enabled_) return;

        std::ostringstream oss;
        oss << "[thread:" << std::this_thread::get_id() << "][conn:" << id_ << "] " << message;
        std::cout << oss.str() << std::endl;
    }
};

std::string createHttpPostRequest(const std::string& host, const std::string& port, const std::string& endpoint, const std::string& message) {
    std::string request = "POST " + endpoint + " HTTP/1.1\r\n";
    request += "Host: " + host + ":" + port + "\r\n";
    request += "Content-Type: application/json\r\n";
    request += "Content-Length: " + std::to_string(message.size()) + "\r\n";
    request += "Connection: keep-alive\r\n\r\n";
    request += message;
    return request;
}

std::atomic<int> Client::total_sent_counter(0);
std::atomic<int> Client::total_received_counter(0);

int main(int argc, char* argv[]) {

    std::string request_json_file;
    std::string hostname;
    std::string port;
    std::string endpoint;
    int timeout;
    int connection_count;
    int thread_count;
    bool enable_logging;
    int delay_between_requests;
    unsigned long volume_per_connection;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "help")
        ("host,h", po::value<std::string>(&hostname)->default_value("127.0.0.1"), "hostname")
        ("port,p", po::value<std::string>(&port)->default_value("4063"), "port")
        ("endpoint,e", po::value<std::string>(&endpoint)->default_value("/openrtb"), "endpoint")
        ("request,r", po::value<std::string>(&request_json_file)->default_value("request.json"), "set request json file")
        ("timeout,to", po::value<int>(&timeout)->default_value(5), "set timeout for waiting for a reply from server (in seconds)")
        ("connections,c", po::value<int>(&connection_count)->default_value(2), "connection count to create to server")
        ("threads,t", po::value<int>(&thread_count)->default_value(2), "connection count to create to server")
        ("logging,l", po::value<bool>(&enable_logging)->default_value(true), "print additional information to the console")
        ("delay,d", po::value<int>(&delay_between_requests)->default_value(100), "delay between requests")
        ("volume,v", po::value<unsigned long>(&volume_per_connection)->default_value(5), "total requests to send on each opened connection")
        ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        cout << desc << endl;
        return 1;
    }

    cout << "Settings:" << endl;
    cout << "Host: " << hostname << endl;
    cout << "Port: " << port << endl;
    cout << "Endpoint: " << endpoint << endl;
    cout << "Request file: "<<  request_json_file << endl;
    cout << "Volume: " << volume_per_connection << " requests sent on each connection" << endl;
    cout << "Response Wait Timeout: " << timeout << " seconds" << endl;
    cout << "Connections: " << connection_count << " total connections to be opened" << endl;
    cout << "Threads: " << thread_count << " total threads pushing to all available connections" << endl;
    cout << "Delay between sending requests: " << delay_between_requests << "(ms) " << endl;
    cout << "Logging enabled: " << enable_logging << ". If true, prints additional information to console" << endl;
    cout << "Start sending ..." << endl;

    try {
        boost::asio::io_context io_context;
        std::vector<std::shared_ptr<Client>> clients;

        // Create multiple clients (ie. more connections)
        for(int i=0; i < connection_count; ++i){
            clients.push_back(std::make_shared<Client>(io_context, hostname, port, i, timeout, enable_logging));
        }

        boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work(io_context.get_executor());

        // threadpool
        std::vector<std::thread> threads;
        for (int i = 0; i < thread_count; ++i) {  // use threads for sending data on all available connections
            threads.emplace_back([&io_context]() {
                io_context.run();
            });
        }

        ifstream json_file(request_json_file);
        if (!json_file.is_open()) {
            cerr << "Failed to open request json file." << endl;
            return 1;
        }
        string json_data((istreambuf_iterator<char>(json_file)), istreambuf_iterator<char>()); // fancy way of loading a file using iterators (says stack overflow)

        string httpRequest = createHttpPostRequest("127.0.0.1", "4063", endpoint, json_data);

        // load requests onto connections
        for(unsigned long i = 0; i < volume_per_connection; ++i){
            for (auto& client : clients) {
                client->async_send(httpRequest);
            }
            this_thread::sleep_for(chrono::milliseconds(delay_between_requests)); // Simulate delay between starting to reload data on connections
        }

        // Cleanup
        // Release the work guard and allow the buffer to drain then close on its own
        work.reset();

        // Wait for all threads to complete
        for (auto& th : threads) {
            th.join();
        }
    } catch (exception& e) {
        cerr << "Exception: " << e.what() << "\n";
    }
    cout << "Summary:\nTotal Requests Sent: "<< Client::total_sent_counter << "\nTotal Responses Received: "<< Client::total_received_counter << endl;

    cout.flush(); // just in case 
}

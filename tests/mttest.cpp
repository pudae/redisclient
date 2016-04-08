#include <string>
#include <iostream>
#include <thread>
#include <boost/asio/ip/address.hpp>
#include <boost/bind.hpp>

#include <redisclient/redisasyncclient.h>

static const std::string redisKey = "unique-redis-key-example";
static const std::string redisValue = "unique-redis-value";

class Worker
{
public:
    Worker(boost::asio::io_service &ioService, RedisAsyncClient &redisClient)
        : ioService(ioService), redisClient(redisClient)
    {}

    void doSet();
    void onConnect(bool connected, const std::string &errorMessage);
    void onSet(const RedisValue &value);
    void onGet(const RedisValue &value);

private:
    boost::asio::io_service &ioService;
    RedisAsyncClient &redisClient;
};

void Worker::onConnect(bool connected, const std::string &errorMessage)
{
    if( !connected )
    {
        std::cerr << "Can't connect to redis: " << errorMessage;
    }
    else
    {
        for (int i = 0; i < 100; ++i)
            redisClient.command("SET",  redisKey, redisValue,
                                boost::bind(&Worker::onSet, this, _1));
    }
}

void Worker::onSet(const RedisValue &value)
{
    if( value.toString() == "OK" )
    {
        redisClient.command("GET",  redisKey,
                            boost::bind(&Worker::onGet, this, _1));
    }
    else
    {
        std::cerr << "Invalid value from redis: " << value.toString() << std::endl;
    }
}

void Worker::onGet(const RedisValue &value)
{
    if( value.toString() != redisValue )
    {
        std::cerr << "Invalid value from redis: " << value.toString() << std::endl;
    }
    else
    {
        redisClient.command("GET",  redisKey,
                            boost::bind(&Worker::onGet, this, _1));
    }
}


int main(int, char **)
{
    const char *address = "10.30.164.228";
    const int port = 6379;

    boost::asio::io_service ioService;
    RedisAsyncClient client(ioService);
    Worker worker(ioService, client);
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(address), port);

    client.asyncConnect(endpoint, boost::bind(&Worker::onConnect, &worker, _1, _2));

    std::vector<std::thread> thr;
    for (int i = 0; i < 10; ++i)
    {
      thr.emplace_back(std::thread{ [&]() { ioService.run(); } });
    }

    ioService.run();

    return 0;
}

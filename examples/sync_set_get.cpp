#include <string>
#include <vector>
#include <iostream>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/address.hpp>
#include <redisclient/redissyncclient.h>

int main(int, char **)
{
    boost::asio::ip::address address = boost::asio::ip::address::from_string("127.0.0.1");
    const unsigned short port = 6379;

    boost::asio::io_service ioService;
    RedisSyncClient redis(ioService);
    std::string errmsg;

    if( !redis.connect(address, port, errmsg) )
    {
        std::cerr << "Can't connect to redis: " << errmsg << std::endl;
        return EXIT_FAILURE;
    }

    RedisValue result;

    result = redis.command("SET", "key", "value");

    if( result.isError() )
    {
        std::cerr << "SET error: " << result.toString() << "\n";
        return EXIT_FAILURE;
    }

    result = redis.command("GET", "key");

    if( result.isOk() )
    {
        std::cout << "GET: " << result.toString() << "\n";
        return EXIT_SUCCESS;
    }
    else
    {
        std::cerr << "GET error: " << result.toString() << "\n";
        return EXIT_FAILURE;
    }
}

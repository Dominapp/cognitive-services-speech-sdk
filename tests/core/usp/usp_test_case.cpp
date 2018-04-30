//
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.md file in the project root for full license information.
//

#include <chrono>
#include <thread>
#include <random>
#include <string>

#include "test_utils.h"
#include "usp.h"

using namespace std;
using namespace Microsoft::CognitiveServices::Speech;

class UspClient : public USP::Callbacks {
public:
    UspClient(USP::EndpointType endpoint = USP::EndpointType::BingSpeech, 
        USP::RecognitionMode mode = USP::RecognitionMode::Interactive)
    {
        auto client = USP::Client(*this, endpoint)
            .SetRecognitionMode(mode)
            .SetAuthentication(USP::AuthenticationType::SubscriptionKey, Keys::Speech);
        if (!Config::Endpoint.empty()) 
        {
            client.SetEndpointType(USP::EndpointType::Custom).SetEndpointUrl(Config::Endpoint);
        }

        m_connection = client.Connect();
    }

    virtual void OnError(const std::string& error) override
    {
        FAIL(error);
    }

    ~UspClient() {
        m_connection.reset();
    }
    
    template <class T>
    void WriteAudio(T* buffer, size_t size) {
        m_connection->WriteAudio(reinterpret_cast<const uint8_t*>(buffer), size);
    }

private:
    USP::ConnectionPtr m_connection;
};


TEST_CASE("USP is properly functioning", "[usp]")
{
    SECTION("usp can be initialized, connected and closed")
    {
        UspClient client;
        (void)(client);
    }

    wstring input_file(L"tests/input/whatstheweatherlike.wav");
    REQUIRE(exists(input_file));

    SECTION("usp can be used to upload binary data")
    {
        string dummy = "RIFF1234567890";
        UspClient client;
        client.WriteAudio(dummy.data(), dummy.length());
    }

    random_engine rnd(12345);
    size_t buffer_size_8k = 1 << 13;
    vector<char> buffer(buffer_size_8k);

    SECTION("usp can be used to upload audio from file")
    {
        UspClient client;
        auto is = get_stream(input_file);

        while (is) {
            auto size_to_read = max(size_t(1 << 10), rnd() % buffer_size_8k);
            is.read(buffer.data(), size_to_read);
            auto bytesRead = (size_t)is.gcount();
            client.WriteAudio(buffer.data(), bytesRead);
            std::this_thread::sleep_for(std::chrono::milliseconds(rnd() % 100));
        }
    }

    SECTION("usp can toggled on/off multiple times in a row")
    {
        for (unsigned int i = 10; i > 0; i--) 
        {
            UspClient client;
            auto is = get_stream(input_file);
            while (is && (rnd()%i < i>>1)) {
                is.read(buffer.data(), buffer_size_8k);
                auto bytesRead = (size_t)is.gcount();
                client.WriteAudio(buffer.data(), bytesRead);
                std::this_thread::sleep_for(std::chrono::milliseconds(rnd() % 100));
            }
        }
    }

    SECTION("several usp clients can coexist peacefully")
    {
        int num_handles = 10;
        vector<UspClient> clients(num_handles);

        auto is = get_stream(input_file);
        is.read(buffer.data(), buffer_size_8k);
        REQUIRE(is.good());

        for (int i = 0; i < num_handles; i++)
        {
            auto bytesRead = (size_t)is.gcount();
            clients[i].WriteAudio(buffer.data(), bytesRead);
        }

        while (is) {
            auto size_to_read = max(size_t(1 << 10), rnd() % buffer_size_8k);
            is.read(buffer.data(), size_to_read);
            auto bytesRead = (size_t)is.gcount();
            clients[rnd() % num_handles].WriteAudio(buffer.data(), bytesRead);
            std::this_thread::sleep_for(std::chrono::milliseconds(rnd() % 100));
        }
    }
}


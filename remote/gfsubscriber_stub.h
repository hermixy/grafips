#pragma once

#include <string>
#include <vector>
#include <mutex>

#include "os/gfsocket.h"
#include "gfisubscriber.h"
#include "os/gftraits.h"

namespace GrafipsProto
{
    class SubscriberInvocation;
}

namespace Grafips
{

    class SubscriberStub : public SubscriberInterface,
                           NoCopy, NoAssign, NoMove
    {
      public:
        SubscriberStub(const std::string &address, int port);
        ~SubscriberStub();
        void Clear(int id);
        void OnMetric(const DataSet &d);
        void OnDescriptions(const std::vector<MetricDescription> &descriptions);
        void Flush() const;
      private:
        void WriteMessage(const GrafipsProto::SubscriberInvocation&m) const;
        mutable Socket m_socket;
        mutable std::vector<unsigned char> m_buf;
        mutable std::mutex m_protect;
    };

}

#pragma once

#include "Serialization.h"
#include "Mapping.h"

namespace Mapping
{
    class Group : public IMapper
    {
    public:
        Group() = default;
        Group(std::vector<IMapperPtr> mappers);

        virtual std::optional<StringDescription> ToString() const override;
        virtual void Map(const X360::Controller& from, const std::atomic_bool* keyboard, N64::Controller& to) override;
        virtual YAML::Node Serialize() const;

        std::vector<IMapperPtr> mappers_;

        bool prevOn_ = false;
        int curMapper_ = 0;
    };

    using GroupPtr = std::shared_ptr<Group>;
}

namespace YAML
{
    template<>
    struct convert<Mapping::GroupPtr>
    {
        static bool decode(const Node& node, Mapping::GroupPtr&);
    };
}

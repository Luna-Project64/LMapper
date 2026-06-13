#pragma once

#include "utils.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "ControllerInterface.h"
#include "X360Controller.h"
#include "N64Controller.h"

namespace Mapping
{
    class IMapper : public Serialization::ISerializable
    {
    public:
        struct StringDescription
        {
            std::string type;
            std::string from;
            std::string to;
        };

        virtual std::optional<StringDescription> ToString() const = 0;
        virtual void Map(const X360::Controller& from, const std::atomic_bool* keyboard, N64::Controller& to) = 0;
    };
    using IMapperPtr = std::shared_ptr<IMapper>;

    namespace Analog
    {
        template<typename FromOffsetT, typename FromStickT, typename ToOffsetT, typename ToStickT>
        class LinearMapper final : public IMapper
        {
        public:
            using Deadzoner = std::optional<ControllerInterface::Deadzoner>;
            using FromConverter = ControllerInterface::LinearConverter<FromOffsetT, FromStickT>;
            using ToConverter = ControllerInterface::LinearConverter<ToOffsetT, ToStickT>;

            LinearMapper(FromConverter, ToConverter, Deadzoner);

            virtual std::optional<StringDescription> ToString() const override;
            virtual void Map(const X360::Controller& from, const std::atomic_bool* keyboard, N64::Controller& to) override;
            virtual YAML::Node Serialize() const override;

        private:
            static const std::string name_;
            FromConverter fromConverter_;
            ToConverter toConverter_;

            const Deadzoner deadzoner_;
        };

        template<typename FromOffsetT, typename FromStickT, typename ToOffsetT, typename ToStickT>
        class BilinearMapper final : public IMapper
        {
        public:
            using AngleLimiter      = std::optional<ControllerInterface::AngleDeadzoner>;
            using Stretcher         = std::optional<ControllerInterface::BilinearDiagonalStretcher>;
            using Deadzoner         = std::optional<ControllerInterface::Deadzoner>;
            using BilinearDeadzoner = std::optional<ControllerInterface::BilinearDeadzoner>;
            using FromConverter     = ControllerInterface::LinearConverter<FromOffsetT, FromStickT>;
            using ToConverter       = ControllerInterface::LinearConverter<ToOffsetT, ToStickT>;

            BilinearMapper(FromConverter fX, FromConverter fY, ToConverter tX, ToConverter tY, Stretcher, Deadzoner, BilinearDeadzoner, AngleLimiter);
            
            virtual std::optional<StringDescription> ToString() const override;
            virtual void Map(const X360::Controller& from, const std::atomic_bool* keyboard, N64::Controller& to) override;
            virtual YAML::Node Serialize() const override;

        private:
            static const std::string name_;
            const FromConverter fromConverters_[2];
            const ToConverter toConverters_[2];

            const Stretcher stretcher_;
            const Deadzoner deadzoner_;
            const BilinearDeadzoner bilinearDeadzoner_;
            const AngleLimiter angleDeadzone_;
        };

        template<typename FromOffsetT, typename FromStickT, typename ToOffsetT, typename ToStickT>
        using LinearMapperPtr = std::shared_ptr<LinearMapper<FromOffsetT, FromStickT, ToOffsetT, ToStickT>>;

        template<typename FromOffsetT, typename FromStickT, typename ToOffsetT, typename ToStickT>
        using BilinearMapperPtr = std::shared_ptr<BilinearMapper<FromOffsetT, FromStickT, ToOffsetT, ToStickT>>;

        using LinearTriggerMapper = LinearMapper<X360::Triggers, BYTE, N64::Axises, char>;
        using BilinearTriggerMapper = BilinearMapper<X360::Triggers, BYTE, N64::Axises, char>;
        using LinearStickMapper = LinearMapper<X360::Thumbs, SHORT, N64::Axises, char>;
        using BilinearStickMapper = BilinearMapper<X360::Thumbs, SHORT, N64::Axises, char>;

        using LinearTriggerMapperPtr = std::shared_ptr<LinearTriggerMapper>;
        using BilinearTriggerMapperPtr = std::shared_ptr<BilinearTriggerMapper>;
        using LinearStickMapperPtr = std::shared_ptr<LinearStickMapper>;
        using BilinearStickMapperPtr = std::shared_ptr<BilinearStickMapper>;
    }

    namespace Digital
    {
        class Mapper : public IMapper
        {
        public:
            Mapper(X360::IEventPtr from, N64::IModifierPtr to);

            virtual std::optional<StringDescription> ToString() const override;
            virtual void Map(const X360::Controller& from, const std::atomic_bool* keyboard, N64::Controller& to) override;
            virtual YAML::Node Serialize() const override;

        private:
            X360::IEventPtr event_;
            N64::IModifierPtr modifier_;
        };

        using MapperPtr = std::shared_ptr<Mapper>;
    }

    using Mappers = std::vector<IMapperPtr>;
    void Map(const Mappers&, const X360::Controller& from, const std::atomic_bool* keyboard, N64::Controller& to);
}

namespace YAML
{
    template<>
    struct convert<Mapping::IMapperPtr>
    {
        static Node encode(const Mapping::IMapperPtr&);
        static bool decode(const Node& node, Mapping::IMapperPtr&);
    };

    template<typename FromOffsetT, typename FromStickT, typename ToOffsetT, typename ToStickT>
    struct convert<Mapping::Analog::LinearMapperPtr<FromOffsetT, FromStickT, ToOffsetT, ToStickT>>
    {
        static bool decode(const Node& node, Mapping::Analog::LinearMapperPtr<FromOffsetT, FromStickT, ToOffsetT, ToStickT>&);
    };

    template<typename FromOffsetT, typename FromStickT, typename ToOffsetT, typename ToStickT>
    struct convert<Mapping::Analog::BilinearMapperPtr<FromOffsetT, FromStickT, ToOffsetT, ToStickT>>
    {
        static bool decode(const Node& node, Mapping::Analog::BilinearMapperPtr<FromOffsetT, FromStickT, ToOffsetT, ToStickT>&);
    };

    template<>
    struct convert<Mapping::Digital::MapperPtr>
    {
        static bool decode(const Node& node, Mapping::Digital::MapperPtr&);
    };
}

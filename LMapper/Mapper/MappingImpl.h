#pragma once

#include "Mapping.h"

#include "SerializationImpl.h"

namespace Mapping
{
    namespace Analog
    {
        template<typename FromConverter, typename ToConverter>
        LinearMapper<FromConverter, ToConverter>::LinearMapper(FromConverter from, ToConverter to, Deadzoner dz) : fromConverter_(from), toConverter_(to), deadzoner_(dz) { }

        template<typename FromConverter, typename ToConverter>
        std::optional<Simple::Config> LinearMapper<FromConverter, ToConverter>::ToSimpleConfig() const
        {
            // There is no simple config for linear mapping because n64 does not have any triggers.
            // We assume that the only mapping for axis is going to be the stick itself.
            return std::nullopt;
        }

        template<typename FromConverter, typename ToConverter>
        void LinearMapper<FromConverter, ToConverter>::Map(const X360::Controller& from, const std::atomic_bool*, N64::Controller& to)
        {
            auto fromValPtr = fromConverter_.Get(&from);
            auto toValPtr = toConverter_.Get(&to);

            auto fromValConv = fromConverter_.Convert(*fromValPtr);
            if (deadzoner_)
                deadzoner_->Apply(fromValConv);
            auto toValConv = toConverter_.Convert(fromValConv);

            *toValPtr = toValConv;
        }

        template<typename FromConverter, typename ToConverter>
        const std::string LinearMapper<FromConverter, ToConverter>::name_ = "linear";

        template<typename FromConverter, typename ToConverter>
        YAML::Node LinearMapper<FromConverter, ToConverter>::Serialize() const
        {
            YAML::Node node;
            node["type"] = name_;
            node["from"] = fromConverter_;
            node["to"] = toConverter_;
            if (deadzoner_)
                node["deadzone"] = *deadzoner_;
            return node;
        }

        static inline std::optional<Simple::FromStick> toSimpleFromStick(Simple::StickAxis x, Simple::StickAxis y)
        {
            if (x == Simple::StickAxis::LeftX && y == Simple::StickAxis::LeftY)
				return Simple::FromStick::Left;

            if (x == Simple::StickAxis::RightX && y == Simple::StickAxis::RightY)
                return Simple::FromStick::Right;

            return std::nullopt;
		}

        template<typename FromConverter, typename ToConverter>
        std::optional<Simple::Config> BilinearMapper<FromConverter, ToConverter>::ToSimpleConfig() const
        {
            if (bilinearDeadzoner_)
                return std::nullopt;

			auto fromXDesc = fromConverters_[0].ToSimpleStickFrom();
            if (!fromXDesc)
				return std::nullopt;

            auto fromYDesc = fromConverters_[1].ToSimpleStickFrom();
			if (!fromYDesc)
				return std::nullopt;

            auto toXDesc = toConverters_[0].ToSimpleRangeTo();
			if (!toXDesc)
                return std::nullopt;

			auto toYDesc = toConverters_[1].ToSimpleRangeTo();
            if (!toYDesc)
				return std::nullopt;

            if (*toXDesc != *toYDesc)
				return std::nullopt;

			auto stick = toSimpleFromStick(*fromXDesc, *fromYDesc);
            if (!stick)
                return std::nullopt;

            float deadzone = 0.f;
            bool angleDeadzoneWithDiagonals = false;
            float angleDeadzone = 0.f;
            float stretcher = 0.f;

            if (deadzoner_)
            {
                deadzone = deadzoner_->GetSize();
            }
            if (angleDeadzone_)
            {
				auto simple8Dir = angleDeadzone_->UsesSimple8Dir();
                if (!simple8Dir)
					return std::nullopt;

				angleDeadzoneWithDiagonals = *simple8Dir;
				angleDeadzone = angleDeadzone_->GetSize();
            }
            if (stretcher_)
            {
                auto slope = stretcher_->GetSimpleSlope();
                if (!slope)
                    return std::nullopt;

                stretcher = *slope;
            }

            return Simple::StickMapping{ *stick, *toXDesc /*=toYDesc*/, deadzone, angleDeadzoneWithDiagonals, angleDeadzone, stretcher };
        }

        template<typename FromConverter, typename ToConverter>
        void BilinearMapper<FromConverter, ToConverter>::Map(const X360::Controller& from, const std::atomic_bool*, N64::Controller& to)
        {
            float tmpValues[2];
            for (int i = 0; i < 2; i++)
            {
                auto& converter = fromConverters_[i];
                auto fromValPtr = converter.Get(&from);
                tmpValues[i] = converter.Convert(*fromValPtr);
            }

            if (deadzoner_)
                deadzoner_->Apply(tmpValues);
            if (bilinearDeadzoner_)
                bilinearDeadzoner_->Apply(tmpValues);
            if (stretcher_)
                stretcher_->Stretch(tmpValues[0], tmpValues[1]);
            if (angleDeadzone_)
                angleDeadzone_->Apply(tmpValues);

            for (int i = 0; i < 2; i++)
            {
                auto& converter = toConverters_[i];
                auto toValPtr = converter.Get(&to);
                *toValPtr = converter.Convert(tmpValues[i]);
            }
        }

        template<typename FromConverter, typename ToConverter>
        BilinearMapper<FromConverter, ToConverter>::BilinearMapper(FromConverter fX, FromConverter fY, ToConverter tX, ToConverter tY, Stretcher s, Deadzoner d, BilinearDeadzoner bd, AngleLimiter al)
            : fromConverters_{ fX, fY }, toConverters_{ tX, tY }, stretcher_(s), deadzoner_(d), bilinearDeadzoner_(bd), angleDeadzone_(al) 
        { }

        template<typename FromConverter, typename ToConverter>
        const std::string BilinearMapper<FromConverter, ToConverter>::name_ = "bilinear";

        template<typename FromConverter, typename ToConverter>
        YAML::Node BilinearMapper<FromConverter, ToConverter>::Serialize() const
        {
            YAML::Node node;
            node["type"] = name_;
            node["fromX"] = fromConverters_[0];
            node["fromY"] = fromConverters_[1];
            node["toX"] = toConverters_[0];
            node["toY"] = toConverters_[1];
            if (stretcher_)
                node["stretcher"] = *stretcher_;
            if (deadzoner_)
                node["deadzone"] = *deadzoner_;
            if (bilinearDeadzoner_)
                node["deadzone"] = *bilinearDeadzoner_;
            if (angleDeadzone_)
                node["angleDeadzone"] = *angleDeadzone_;
            return node;
        }
    }
}

namespace YAML
{
    template<typename FromConverter, typename ToConverter>
    bool convert<Mapping::Analog::LinearMapperPtr<FromConverter, ToConverter>>::decode(const Node& node, Mapping::Analog::LinearMapperPtr<FromConverter, ToConverter>& mapper)
    {
        if (!node.IsMap())
            return true;

        auto fromNode = node["from"];
        auto toNode = node["to"];
        auto deadzoneNode = node["deadzone"];

        std::optional<ControllerInterface::Deadzoner> deadzoner;
        if (deadzoneNode)
            deadzoner = deadzoneNode.as<ControllerInterface::Deadzoner>();

        if (!fromNode || !toNode)
            return false;
        
        auto from = fromNode.as<FromConverter>();
        auto to = toNode.as<ToConverter>();

        mapper = std::make_unique<Mapping::Analog::LinearMapper<FromConverter, ToConverter>>(std::move(from), std::move(to), std::move(deadzoner));
        return true;
    }

    template<typename FromConverter, typename ToConverter>
    bool convert<Mapping::Analog::BilinearMapperPtr<FromConverter, ToConverter>>::decode(const Node& node, Mapping::Analog::BilinearMapperPtr<FromConverter, ToConverter>& mapper)
    {
        if (!node.IsMap())
            return true;

        auto fromXNode = node["fromX"];
        auto fromYNode = node["fromY"];
        auto toXNode = node["toX"];
        auto toYNode = node["toY"];
        auto stretcherNode = node["stretcher"];
        auto deadzoneNode = node["deadzone"];
        auto angleDeadzoneNode = node["angleDeadzone"];

        if (!fromXNode || !fromYNode || !toXNode || !toYNode)
            return false;

        auto fromX = fromXNode.as<FromConverter>();
        auto fromY = fromYNode.as<FromConverter>();
        auto toX = toXNode.as<ToConverter>();
        auto toY = toYNode.as<ToConverter>();
        std::optional<ControllerInterface::BilinearDiagonalStretcher> stretcher;
        if (stretcherNode)
            stretcher = stretcherNode.as<ControllerInterface::BilinearDiagonalStretcher>();

        std::optional<ControllerInterface::Deadzoner> deadzoner;
        std::optional<ControllerInterface::BilinearDeadzoner> bilinearDeadzoner;
        if (deadzoneNode)
        {
            if (deadzoneNode.IsScalar())
            {
                deadzoner = deadzoneNode.as<ControllerInterface::Deadzoner>();
            }
            else
            {
                bilinearDeadzoner = deadzoneNode.as<ControllerInterface::BilinearDeadzoner>();
            }
        }

        std::optional<ControllerInterface::AngleDeadzoner> angleDeadzone;
        if (angleDeadzoneNode)
            angleDeadzone = angleDeadzoneNode.as<ControllerInterface::AngleDeadzoner>();

        mapper = std::make_unique<Mapping::Analog::BilinearStickMapper>(std::move(fromX), std::move(fromY), std::move(toX), std::move(toY), std::move(stretcher), std::move(deadzoner), std::move(bilinearDeadzoner), std::move(angleDeadzone));
        return true;
    }
}

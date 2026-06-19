#include "ControllerInterface.h"

#include <yaml-cpp/yaml.h>

#include "SerializationImpl.h"

#ifndef M_PI
#    define M_PI 3.14159265358979323846f
#endif

namespace ControllerInterface
{
    static inline float slope(float d) { return (1 - d) / d; }
    static inline float unslope(float s) { return 1 / (1 + s); }

    BilinearDiagonalStretcher::BilinearDiagonalStretcher() : BilinearDiagonalStretcher(1, 1, 1) {}

    BilinearDiagonalStretcher::BilinearDiagonalStretcher(float fromDiagonal, float toDiagonal, float dampingPower)
        : fromSlope_(slope(fromDiagonal))
        , toSlope_(slope(toDiagonal))
        , power_(dampingPower) {
    }

    void BilinearDiagonalStretcher::Stretch(float& x, float& y) const
    {
        if (x == 0 && y == 0)
            return;

        float X = fabs(x);
        float Y = fabs(y);
        if (Y > X)
            std::swap(X, Y);

        float fromAvg = X + fromSlope_ * Y;
        float toAvg = X + toSlope_ * Y;

        float fullMultiplier = fromAvg / toAvg;
        float addition = fullMultiplier - 1;
        float dampening = pow(fromAvg, power_);

        float multiplier = addition * dampening;

        x *= multiplier;
        y *= multiplier;
    }

#define EPSILON 0.0001f
    static bool similar(float a, float b)
    {
        return fabs(a - b) < EPSILON;
    }

    std::optional<float> BilinearDiagonalStretcher::GetSimpleSlope() const
    {
        if (!similar(toSlope_, 0.70710678118f)
            || !similar(power_, 1.f))
            return std::nullopt;

        if (similar(toSlope_, fromSlope_))
            return 0.f;

        return fromSlope_;
    }

    YAML::Node BilinearDiagonalStretcher::Serialize() const
    {
        YAML::Node node;
        node["fromDiagonal"] = unslope(fromSlope_);
        node["toDiagonal"] = unslope(toSlope_);
        node["power"] = power_;
        return node;
    }

    Deadzoner::Deadzoner(float size) : size_(size)
    {
    }

    void Deadzoner::Apply(float& v) const
    {
        if (fabs(v) < size_)
            v = 0;
    }

    YAML::Node Deadzoner::Serialize() const
    {
        return YAML::Node{ size_ };
    }

    YAML::Node AxisDeadzoneSize::Serialize() const
    {
        YAML::Node node;
        node.push_back(negative);
        node.push_back(positive);
        return node;
    }

    BilinearDeadzoner::BilinearDeadzoner(AxisDeadzoneSize size[2]) : size_{ size[0], size[1] }
    {
    }

    void BilinearDeadzoner::Apply(float(&arr)[2]) const
    {
        float sum = 0;
        for (int i = 0; i < 2; i++)
        {
            auto size = arr[i] > 0 ? size_[i].positive : size_[i].negative;
            if (size == 0.f)
            {
                // with 0 deadzone, only exact 0 can be deadzoned (0), otherwise no axis can be deadzones (inf)
                if (arr[i] == 0.f)
                {
                    continue;
                }
                else
                {
                    return;
                }
            }

            auto val = arr[i] / size;
            sum += val * val;
        }

        if (sum < 1)
            for (int i = 0; i < 2; i++)
                arr[i] = 0;
    }

    YAML::Node BilinearDeadzoner::Serialize() const
    {
        YAML::Node ret;
        ret["x"] = size_[0].Serialize();
        ret["y"] = size_[1].Serialize();
        return ret;
    }

    AngleDeadzoner::AngleDeadzoner() : size_(0), count_(0)
    {
    }

    AngleDeadzoner::AngleDeadzoner(int count, float size)
        : count_(count)
        , size_(size)
    {
    }

    void AngleDeadzoner::Apply(float(&arr)[2]) const
    {
        auto phi = atan2f(arr[1], arr[0]);

        auto anglePart = 2 * M_PI / count_;
        auto deadzoneAnglePart = anglePart * size_;
        auto roundedPhi = roundf(phi / anglePart) * anglePart;
        auto distPhi = fabsf(roundedPhi - phi);

        if (distPhi < deadzoneAnglePart)
        {
            auto r = sqrtf(arr[0] * arr[0] + arr[1] * arr[1]);
            arr[0] = r * cosf(roundedPhi);
            arr[1] = r * sinf(roundedPhi);
        }
    }

    std::optional<bool> AngleDeadzoner::UsesSimple8Dir() const
    {
        switch (count_)
        {
        case 4:
            return false;
        case 8:
            return true;
        default:
            return std::nullopt;
        }
    }

    YAML::Node AngleDeadzoner::Serialize() const
    {
        YAML::Node ret;
        ret["directions"] = count_;
        ret["deadzone"] = size_;
        return ret;
    }
}

namespace YAML
{
    const std::map<std::string, ControllerInterface::AxisComparerType> convert<ControllerInterface::AxisComparerType>::names{
        { "More", ControllerInterface::AxisComparerType::More },
        { "Less", ControllerInterface::AxisComparerType::Less },
    };

    Node convert<ControllerInterface::AxisComparerType>::encode(const ControllerInterface::AxisComparerType& t)
    {
        return Serialization::EnumSerializer<ControllerInterface::AxisComparerType>::Encode(names, t);
    }

    bool convert<ControllerInterface::AxisComparerType>::decode(const Node& node, ControllerInterface::AxisComparerType& t)
    {
        return Serialization::EnumSerializer<ControllerInterface::AxisComparerType>::Decode(names, node, t);
    }

    Node convert<ControllerInterface::BilinearDiagonalStretcher>::encode(const ControllerInterface::BilinearDiagonalStretcher& me)
    {
        return me.Serialize();
    }

    bool convert<ControllerInterface::BilinearDiagonalStretcher>::decode(const Node& node, ControllerInterface::BilinearDiagonalStretcher& stretcher)
    {
        if (!node.IsMap())
            return false;

        auto fromSlopeNode = node["fromDiagonal"];
        auto toSlopeNode = node["toDiagonal"];
        auto powerNode = node["power"];

        if (!fromSlopeNode || !toSlopeNode || !powerNode)
            return false;

        auto fromSlope = fromSlopeNode.as<float>();
        auto toSlope = toSlopeNode.as<float>();
        auto power = powerNode.as<float>();

        stretcher = ControllerInterface::BilinearDiagonalStretcher(fromSlope, toSlope, power);
        return true;
    }

    Node convert<ControllerInterface::Deadzoner>::encode(const ControllerInterface::Deadzoner& me)
    {
        return me.Serialize();
    }

    bool convert<ControllerInterface::Deadzoner>::decode(const Node& node, ControllerInterface::Deadzoner& dz)
    {
        if (!node.IsScalar())
            return false;

        auto size = node.as<float>();
        dz = ControllerInterface::Deadzoner(size);

        return true;
    }

    Node convert<ControllerInterface::AxisDeadzoneSize>::encode(const ControllerInterface::AxisDeadzoneSize& me)
    {
        return me.Serialize();
    }

    bool convert<ControllerInterface::AxisDeadzoneSize>::decode(const Node& node, ControllerInterface::AxisDeadzoneSize& me)
    {
        if (!node.IsSequence() && node.size() != 2)
            return false;

        me.negative = node[0].as<float>();
        me.positive = node[1].as<float>();

        return true;
    }

    Node convert<ControllerInterface::BilinearDeadzoner>::encode(const ControllerInterface::BilinearDeadzoner& me)
    {
        return me.Serialize();
    }

    bool convert<ControllerInterface::BilinearDeadzoner>::decode(const Node& node, ControllerInterface::BilinearDeadzoner& me)
    {
        if (!node.IsMap())
            return false;

        ControllerInterface::AxisDeadzoneSize sizes[] = {
            node["x"].as<ControllerInterface::AxisDeadzoneSize>(),
            node["y"].as<ControllerInterface::AxisDeadzoneSize>()
        };
        me = ControllerInterface::BilinearDeadzoner(sizes);
        return true;
    }

    Node convert<ControllerInterface::AngleDeadzoner>::encode(const ControllerInterface::AngleDeadzoner& me)
    {
        return me.Serialize();
    }

    bool convert<ControllerInterface::AngleDeadzoner>::decode(const Node& node, ControllerInterface::AngleDeadzoner& al)
    {
        if (!node.IsMap())
            return false;

        auto dz = node["deadzone"].as<float>();
        auto count = node["directions"].as<int>();
        al = ControllerInterface::AngleDeadzoner(count, dz);

        return true;
    }
}
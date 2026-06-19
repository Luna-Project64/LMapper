#pragma once

#include "utils.h"
#include "Serialization.h"
#include "SimpleConfig.h"

#include <map>
#include <memory>
#include <optional>
#include <string>

namespace YAML
{
    class Node;
}

namespace Simple
{
    enum class StickAxis
    {
        LeftX,
        RightX,
        LeftY,
        RightY,
    };
}

namespace ControllerInterface
{
    template<typename ButtonT>
    class Button : public virtual Serialization::ISerializable
    {
    public:
        Button(ButtonT button);

        virtual YAML::Node Serialize() const override;

        template<typename HolderT>
        void Apply(HolderT& controller) const;

        template<typename HolderT>
        bool Applied(const HolderT& controller) const;

        bool Applied(const std::atomic_bool*) const;

    protected:
        const ButtonT button_;
    };

    enum class AxisComparerType
    {
        Less,
        More,
    };

    template<typename T>
    class AxisComparer final : public virtual Serialization::ISerializable
    {
    public:
        using Type = AxisComparerType;
        AxisComparer(Type t);

        virtual YAML::Node Serialize() const override;

        bool operator()(const T a, const T b) const;

        const Type type_;
    };

    // OffsetT must have size_t as a Base
    template<typename AxisT, typename OffsetT>
    class Axis : public virtual Serialization::ISerializable
    {
    public:
        Axis(AxisT axis, OffsetT offset);
        virtual YAML::Node Serialize() const override;

        void Apply(void* ptr) const;

    protected:
        static const char name_[];
        AxisT axis_;
        OffsetT offset_;
    };

    template<typename AxisT, typename OffsetT>
    class AxisEvent : public virtual Serialization::ISerializable
    {
    public:
        AxisEvent(AxisT axis, AxisComparerType compar, OffsetT offset);
        virtual YAML::Node Serialize() const override;

        bool Applied(const void* ptr) const;

    protected:
        static const char name_[];
        AxisT axis_;
        OffsetT offset_;
        AxisComparer<AxisT> comparer_;
    };

    template<typename ControllerT>
    class IEvent : public virtual Serialization::ISerializable
    {
    public:
        virtual std::optional<Simple::FromButton> ToSimpleButton() const = 0;
        virtual bool Happened(const ControllerT&, const std::atomic_bool* keyboard) const = 0;
    };

    template<typename ControllerT>
    class IModifier : public virtual Serialization::ISerializable
    {
    public:
        virtual std::optional<Simple::ToButton> ToSimpleButton() const = 0;
        virtual void Alter(ControllerT&) const = 0;
    };

    template<typename ControllerT>
    using IEventPtr = std::shared_ptr<IEvent<ControllerT>>;

    template<typename ControllerT>
    using IModifierPtr = std::shared_ptr<IModifier<ControllerT>>;

    enum class SimpleStick
    {
        Left,
        Right,
    };

    // Converter from anything to [-1; 1] floating point axis
    template<typename OffsetT, typename StickT>
    class LinearConverter : public Serialization::ISerializable
    {
    public:
        LinearConverter() = default;
        LinearConverter(OffsetT offset, StickT center, StickT max);

        StickT* Get(void* ptr) const { return fieldin(ptr, StickT, offset_); }
        const StickT* Get(const void* ptr) const { return fieldin(ptr, const StickT, offset_); }

        // convert from StickT to [-1; 1]
        float Convert(StickT) const;

        // convert from [-1; 1] to StickT
        StickT Convert(float) const;

        virtual YAML::Node Serialize() const override;

    protected:
        OffsetT offset_;
        StickT center_;
        StickT maxval_;
    };

    // Symmetric axial stretcher
    // both sticks are assumed to be mapped in [-1; 1]
    class BilinearDiagonalStretcher final : public Serialization::ISerializable
    {
    public:
        BilinearDiagonalStretcher();
        BilinearDiagonalStretcher(float fromDiagonal, float toDiagonal, float dampingPower = 1);

        void Stretch(float&, float&) const;

        virtual YAML::Node Serialize() const override;
        std::optional<float> GetSimpleSlope() const;

    protected:
        float fromSlope_;
        float toSlope_;
        float power_;
    };

    class Deadzoner final : public Serialization::ISerializable
    {
    public:
        Deadzoner() = default;
        Deadzoner(float size);

        void Apply(float&) const;

        template<size_t SIZE>
        void Apply(float(&arr)[SIZE]) const;

        virtual YAML::Node Serialize() const override;
        float GetSize() const { return size_; }

    protected:
        float size_;
    };

    struct AxisDeadzoneSize
    {
        float positive;
        float negative;

        YAML::Node Serialize() const;
    };

    class BilinearDeadzoner final : public Serialization::ISerializable
    {
    public:
        BilinearDeadzoner() = default;
        BilinearDeadzoner(AxisDeadzoneSize[2]);

        void Apply(float(&arr)[2]) const;

        virtual YAML::Node Serialize() const override;

    protected:
        AxisDeadzoneSize size_[2];
    };

    class AngleDeadzoner final : public Serialization::ISerializable
    {
    public:
        AngleDeadzoner();
        AngleDeadzoner(int count, float size);

        void Apply(float(&arr)[2]) const;

        virtual YAML::Node Serialize() const override;

        std::optional<bool> UsesSimple8Dir() const;
        float GetSize() const { return size_; }

    protected:
        int count_;
        float size_;
    };
}

namespace YAML
{
    template<>
    struct convert<ControllerInterface::AxisComparerType>
    {
        static const std::map<std::string, ControllerInterface::AxisComparerType> names;

        static Node encode(const ControllerInterface::AxisComparerType&);
        static bool decode(const Node& node, ControllerInterface::AxisComparerType&);
    };

    template<typename T>
    struct convert<ControllerInterface::AxisComparer<T>>
    {
        static Node encode(const ControllerInterface::AxisComparer<T>&);
        static bool decode(const Node& node, ControllerInterface::AxisComparer<T>&);
    };

    template<typename T>
    struct convert<ControllerInterface::IEventPtr<T>>
    {
        static Node encode(const ControllerInterface::IEventPtr<T>&);
    };

    template<typename T>
    struct convert<ControllerInterface::IModifierPtr<T>>
    {
        static Node encode(const ControllerInterface::IModifierPtr<T>&);
    };

    template<typename T, typename S>
    struct convert<ControllerInterface::LinearConverter<T, S>>
    {
        static Node encode(const ControllerInterface::LinearConverter<T, S>&);
        template<typename I>
        static bool decode(const Node& node, I&);
    };

    template<>
    struct convert<ControllerInterface::BilinearDiagonalStretcher>
    {
        static Node encode(const ControllerInterface::BilinearDiagonalStretcher&);
        static bool decode(const Node& node, ControllerInterface::BilinearDiagonalStretcher&);
    };

    template<>
    struct convert<ControllerInterface::Deadzoner>
    {
        static Node encode(const ControllerInterface::Deadzoner&);
        static bool decode(const Node& node, ControllerInterface::Deadzoner&);
    };

    template<>
    struct convert<ControllerInterface::AxisDeadzoneSize>
    {
        static Node encode(const ControllerInterface::AxisDeadzoneSize&);
        static bool decode(const Node& node, ControllerInterface::AxisDeadzoneSize&);
    };

    template<>
    struct convert<ControllerInterface::BilinearDeadzoner>
    {
        static Node encode(const ControllerInterface::BilinearDeadzoner&);
        static bool decode(const Node& node, ControllerInterface::BilinearDeadzoner&);
    };

    template<>
    struct convert<ControllerInterface::AngleDeadzoner>
    {
        static Node encode(const ControllerInterface::AngleDeadzoner&);
        static bool decode(const Node& node, ControllerInterface::AngleDeadzoner&);
    };
}
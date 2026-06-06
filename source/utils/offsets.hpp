#pragma once
#include <cstdint>
#include <string>

namespace Offsets {

namespace TaskScheduler {
extern std::uint64_t Pointer;
extern std::uint64_t MaxFPS;
}

namespace VisualEngine {
extern std::uint64_t Pointer;
extern std::uint64_t Dimensions;
extern std::uint64_t ViewMatrix;
}

namespace FakeDataModel {
extern std::uint64_t Pointer;
extern std::uint64_t RealDataModel;
}

namespace Instance {
extern std::uint64_t Name;
extern std::uint64_t ChildrenStart;
extern std::uint64_t ChildrenEnd;
extern std::uint64_t Parent;
extern std::uint64_t ClassDescriptor;
extern std::uint64_t ClassName;
extern std::uint64_t AttributeContainer;
extern std::uint64_t AttributeList;
extern std::uint64_t AttributeToNext;
extern std::uint64_t AttributeToValue;
}

namespace Misc {
extern std::uint64_t Value;
extern std::uint64_t AnimationId;
}

namespace DataModel {
extern std::uint64_t GameLoaded;
extern std::uint64_t Workspace;
}

namespace RunService {
extern std::uint64_t HeartbeatTask;
extern std::uint64_t HeartbeatFPS;
}

namespace Workspace {
extern std::uint64_t CurrentCamera;
}

namespace Player {
extern std::uint64_t LocalPlayer;
extern std::uint64_t ModelInstance;
}

namespace Humanoid {
extern std::uint64_t Health;
extern std::uint64_t MaxHealth;
extern std::uint64_t MoveDirection;
}

namespace StatsItem {
extern std::uint64_t Value;
}

namespace Camera {
extern std::uint64_t Rotation;
}

namespace BasePart {
extern std::uint64_t Primitive;
}

namespace Primitive {
extern std::uint64_t Position;
extern std::uint64_t Size;
extern std::uint64_t Rotation;
extern std::uint64_t Flags;
extern std::uint64_t AssemblyLinearVelocity;
}

namespace Model {
extern std::uint64_t PrimaryPart;
}

namespace GuiObject {
extern std::uint64_t Position;
extern std::uint64_t Size;
extern std::uint64_t Visible;
extern std::uint64_t Text;
}

namespace UserInputService {
extern std::uint64_t WindowInputState;
}

namespace WindowInputState {
extern std::uint64_t CurrentTextBox;
}

namespace Lighting {
extern std::uint64_t FogEnd;
}

namespace Atmosphere {
extern std::uint64_t Density;
}

namespace AnimationTrack {
extern std::uint64_t Animation;
extern std::uint64_t TimePosition;
}

namespace Animator {
extern std::uint64_t ActiveAnimations;
}

int Fetched(void);
void Reset(void);
void Fetch(void);

}

namespace FFlagOffsets {
namespace FFlags {
extern std::uint64_t NextGenReplicatorEnabledWrite4;
}
}

#include "sdk.hpp"
#include "../utils/memory.hpp"
#include "../utils/offsets.hpp"
#include "../utils/timer_manager.hpp"
#include "../functions/esp.hpp"
#include "../functions/fly.hpp"

std::uint64_t Character = 0;
std::uint64_t Humanoid = 0;
std::uint64_t Chat = 0;
std::uint64_t Menu = 0;

std::uint64_t SDK::ModuleBase = 0;
Instance SDK::DataModel = Instance{};
Instance SDK::Players = Instance{};
Instance SDK::LocalPlayer = Instance{};
Instance SDK::Character = Instance{};
Instance SDK::Humanoid = Instance{};
Instance SDK::HumanoidRootPart = Instance{};
std::uint64_t SDK::HumanoidRootPartPrimitive = 0;
Instance SDK::GroundSensor = Instance{};
Instance SDK::SpellFrame = Instance{};
Instance SDK::Symbols = Instance{};
std::uint64_t SDK::CanCollide = 0;
std::uint8_t SDK::CanCollideMask = 0;
Instance SDK::Lighting = Instance{};
std::uint64_t SDK::FogEnd = 0;
std::vector<Instance> SDK::Atmosphere{};
std::vector<float> SDK::Density{};
std::uint64_t SDK::GameLoaded = 0;
std::uint64_t SDK::HeartbeatFPS = 0;
std::uint64_t SDK::Ping = 0;
std::uint64_t SDK::TaskSchedulerMaxFPS = 0;
float SDK::fly_speed = 25.f;
bool SDK::fly_enabled = false;
bool SDK::no_fog = false;
bool SDK::hitbox_visual = false;
bool SDK::health_visual = false;
HWND SDK::WindowHandle = nullptr;

static void GetCharacter(void) {
	HANDLE process = mem::get_process_handle(0);
	while (true) {
		if (!process || process == INVALID_HANDLE_VALUE || WaitForSingleObject(process, 0) == WAIT_OBJECT_0) return;
		if (mem::read<std::uint64_t>(SDK::GameLoaded) != 31) return;
		std::uint64_t model = mem::read<std::uint64_t>(SDK::LocalPlayer.address + Offsets::Player::ModelInstance);
		if (model) {
			std::uint64_t object = Instance{ model }.FindFirstChildOfClass("Humanoid").address;
			if (object) {
				::Character = model;
				::Humanoid = object;
				return;
			}
		}
		Sleep(440);
	}
}

static void GetAtmospheres(void) {
	for (const Instance& child : SDK::DataModel.FindFirstChild("ReplicatedStorage").FindFirstChild("SkyBoxes").GetChildren()) {
		if (child.GetInstanceClassName() != "Atmosphere") continue;
		SDK::Atmosphere.push_back(child);
		SDK::Density.push_back(mem::read<float>(child.address + Offsets::Atmosphere::Density));
	}
}

void SDK::Load(void) {
	while (!Offsets::Fetched()) Sleep(440);
	EnumWindows([](HWND hwnd, LPARAM lp) -> BOOL { DWORD wp = 0; GetWindowThreadProcessId(hwnd, &wp); if (wp == (DWORD)(uintptr_t)lp) { SDK::WindowHandle = hwnd; return FALSE; } return TRUE; }, (LPARAM)(uintptr_t)mem::get_process_id());
	SDK::ModuleBase = mem::get_module_address();
	SDK::DataModel = Instance{ mem::read<std::uint64_t>(mem::read<std::uint64_t>(SDK::ModuleBase + Offsets::FakeDataModel::Pointer) + Offsets::FakeDataModel::RealDataModel) };
	SDK::GameLoaded = SDK::DataModel.address + Offsets::DataModel::GameLoaded;
	SDK::Players = SDK::DataModel.FindFirstChildOfClass("Players");
	SDK::LocalPlayer = Instance{ mem::read<std::uint64_t>(SDK::Players.address + Offsets::Player::LocalPlayer) };
	SDK::TaskSchedulerMaxFPS = mem::read<std::uint64_t>(SDK::ModuleBase + Offsets::TaskScheduler::Pointer) + Offsets::TaskScheduler::MaxFPS;
	SDK::HeartbeatFPS = mem::read<std::uint64_t>(SDK::DataModel.FindFirstChildOfClass("RunService").address + Offsets::RunService::HeartbeatTask) + Offsets::RunService::HeartbeatFPS;
	SDK::Ping = SDK::DataModel.FindFirstChild("Stats").FindFirstChild("Network").FindFirstChild("ServerStatsItem").FindFirstChild("Data Ping").address + Offsets::StatsItem::Value;
	GetCharacter();
	SDK::Character = Instance{ ::Character };
	SDK::Humanoid = Instance{ ::Humanoid };
	SDK::HumanoidRootPart = SDK::Character.FindFirstChild("HumanoidRootPart");
	SDK::HumanoidRootPartPrimitive = mem::read<std::uint64_t>(SDK::HumanoidRootPart.address + Offsets::BasePart::Primitive);
	SDK::GroundSensor = SDK::HumanoidRootPart.FindFirstChild("GroundSensor");
	SDK::SpellFrame = SDK::LocalPlayer.FindFirstChild("PlayerGui").FindFirstChild("SpellGui").FindFirstChild("SpellFrame");
	SDK::Symbols = SDK::SpellFrame.FindFirstChild("Symbols");
	::Chat = mem::read<std::uint64_t>(SDK::DataModel.FindFirstChildOfClass("UserInputService").address + Offsets::UserInputService::WindowInputState) + Offsets::WindowInputState::CurrentTextBox;
	::Menu = SDK::DataModel.FindFirstChildOfClass("CoreGui").FindFirstChild("RobloxGui").FindFirstChild("SettingsClippingShield").FindFirstChild("DarkenBackground").address + Offsets::GuiObject::Visible;
	SDK::CanCollide = mem::read<std::uint64_t>(SDK::Character.FindFirstChild("RootCollider").address + Offsets::BasePart::Primitive) + Offsets::Primitive::Flags;
	SDK::CanCollideMask = mem::read<std::uint8_t>(SDK::CanCollide);
	SDK::Lighting = SDK::DataModel.FindFirstChildOfClass("Lighting");
	SDK::FogEnd = SDK::Lighting.address + Offsets::Lighting::FogEnd;
	GetAtmospheres();
}

void SDK::Unload(void) {
	::Character = 0;
	::Humanoid = 0;
	::Chat = 0;
	::Menu = 0;
	SDK::ModuleBase = 0;
	SDK::DataModel = Instance{};
	SDK::Players = Instance{};
	SDK::LocalPlayer = Instance{};
	SDK::Character = Instance{};
	SDK::Humanoid = Instance{};
	SDK::HumanoidRootPart = Instance{};
	SDK::HumanoidRootPartPrimitive = 0;
	SDK::GroundSensor = Instance{};
	SDK::SpellFrame = Instance{};
	SDK::Symbols = Instance{};
	SDK::Lighting = Instance{};
	SDK::FogEnd = 0;
	SDK::Atmosphere.clear();
	SDK::Density.clear();
	SDK::GameLoaded = 0;
	SDK::HeartbeatFPS = 0;
	SDK::Ping = 0;
	SDK::TaskSchedulerMaxFPS = 0;
	SDK::CanCollide = 0;
	SDK::CanCollideMask = 0;
	SDK::WindowHandle = nullptr;
	esp_reset();
	fly_reset();
}

void SDK::ReloadCharacter(void) {
	SDK::Players = SDK::DataModel.FindFirstChildOfClass("Players");
	SDK::LocalPlayer = Instance{ mem::read<std::uint64_t>(SDK::Players.address + Offsets::Player::LocalPlayer) };
	GetCharacter();
	SDK::Character = Instance{ ::Character };
	SDK::Humanoid = Instance{ ::Humanoid };
	SDK::HumanoidRootPart = SDK::Character.FindFirstChild("HumanoidRootPart");
	SDK::HumanoidRootPartPrimitive = mem::read<std::uint64_t>(SDK::HumanoidRootPart.address + Offsets::BasePart::Primitive);
	SDK::GroundSensor = SDK::HumanoidRootPart.FindFirstChild("GroundSensor");
	SDK::SpellFrame = SDK::LocalPlayer.FindFirstChild("PlayerGui").FindFirstChild("SpellGui").FindFirstChild("SpellFrame");
	SDK::Symbols = SDK::SpellFrame.FindFirstChild("Symbols");
	SDK::CanCollide = mem::read<std::uint64_t>(SDK::Character.FindFirstChild("RootCollider").address + Offsets::BasePart::Primitive) + Offsets::Primitive::Flags;
	SDK::CanCollideMask = mem::read<std::uint8_t>(SDK::CanCollide);
	esp_reset();
}

UIActiveState SDK::UIActive(void) {
	static UIActiveState cached{};
	if (task.wait<64>()) {
		cached.chat = mem::read<std::uint64_t>(::Chat) != 0;
		cached.menu = mem::read<bool>(::Menu);
	}
	return cached;
}

WindowState SDK::Window(void) {
	WindowState out{}; out.foreground = false; out.position = { 0, 0 }; out.client_rect = { 0, 0, 0, 0 };
	out.foreground = (GetForegroundWindow() == SDK::WindowHandle);
	RECT rc = {}; POINT pt = { 0, 0 };
	if (SDK::WindowHandle) { GetClientRect(SDK::WindowHandle, &rc); ClientToScreen(SDK::WindowHandle, &pt); out.client_rect = rc; out.position = pt; }
	return out;
}

std::vector<Instance> Instance::GetChildren() const {
	std::vector<Instance> children;
	std::uint64_t start = mem::read<std::uint64_t>(address + Offsets::Instance::ChildrenStart);
	std::uint64_t end = mem::read<std::uint64_t>(start + Offsets::Instance::ChildrenEnd);
	for (std::uint64_t entry = mem::read<std::uint64_t>(start); entry < end; entry += 0x10) {
		std::uint64_t instance = mem::read<std::uint64_t>(entry);
		children.emplace_back(Instance{ instance });
	}
	return children;
}

Instance Instance::FindFirstChild(std::string name) const {
	for (auto& child : GetChildren()) if (child.GetInstanceName() == name) return child;
	return Instance{};
}

Instance Instance::FindFirstChildOfClass(std::string name) const {
	for (auto& child : GetChildren()) if (child.GetInstanceClassName() == name) return child;
	return Instance{};
}

std::string Instance::GetInstanceName() const {
	return mem::read_string(mem::read<std::uint64_t>(address + Offsets::Instance::Name));
}

std::string Instance::GetInstanceClassName() const {
	return mem::read_string(mem::read<std::uint64_t>(mem::read<std::uint64_t>(address + Offsets::Instance::ClassDescriptor) + Offsets::Instance::ClassName));
}

std::string Instance::GetAttribute(const char* name) const {
	std::uint64_t AttributeContainer = mem::read<std::uint64_t>(address + Offsets::Instance::AttributeContainer);
	std::uint64_t AttributeList = mem::read<std::uint64_t>(AttributeContainer + Offsets::Instance::AttributeList);
	std::uintptr_t AttributeToNext = (std::uintptr_t)Offsets::Instance::AttributeToNext;
	for (std::uintptr_t index = 0; index < AttributeToNext * 256; index += AttributeToNext) {
		std::string attribute = mem::read_string(mem::read<std::uint64_t>(AttributeList + index));
		if (attribute != name) { if (attribute.empty()) break; continue; }
		if (attribute == "FloorMaterial") {
			mem::write<std::int32_t>(AttributeList + index + Offsets::Instance::AttributeToValue, 800); return "";
		}
		return mem::read_string(AttributeList + index + Offsets::Instance::AttributeToValue);
	}
	return "";
}

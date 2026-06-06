#include "offsets.hpp"
#include "../json/simdjson.h"
#include "http.hpp"
#include "memory.hpp"
#include <windows.h>
#include <filesystem>
#include <psapi.h>

static bool offsets_initialized = false;

namespace Offsets {
namespace TaskScheduler {
std::uint64_t Pointer = 0;
std::uint64_t MaxFPS = 0;
}

namespace VisualEngine {
std::uint64_t Pointer = 0;
std::uint64_t Dimensions = 0;
std::uint64_t ViewMatrix = 0;
}

namespace FakeDataModel {
std::uint64_t Pointer = 0;
std::uint64_t RealDataModel = 0;
}

namespace Instance {
std::uint64_t Name = 0;
std::uint64_t ChildrenStart = 0;
std::uint64_t ChildrenEnd = 0;
std::uint64_t Parent = 0;
std::uint64_t ClassDescriptor = 0;
std::uint64_t ClassName = 0;
std::uint64_t AttributeContainer = 0;
std::uint64_t AttributeList = 0;
std::uint64_t AttributeToNext = 0;
std::uint64_t AttributeToValue = 0;
}

namespace Misc {
std::uint64_t Value = 0;
std::uint64_t AnimationId = 0;
}

namespace DataModel {
std::uint64_t GameLoaded = 0;
std::uint64_t Workspace = 0;
}

namespace RunService {
std::uint64_t HeartbeatTask = 0;
std::uint64_t HeartbeatFPS = 0;
}

namespace Workspace {
std::uint64_t CurrentCamera = 0;
}

namespace Player {
std::uint64_t LocalPlayer = 0;
std::uint64_t ModelInstance = 0;
}

namespace Humanoid {
std::uint64_t Health = 0;
std::uint64_t MaxHealth = 0;
std::uint64_t MoveDirection = 0;
}

namespace StatsItem {
std::uint64_t Value = 0;
}

namespace Camera {
std::uint64_t Rotation = 0;
}

namespace BasePart {
std::uint64_t Primitive = 0;
}

namespace Primitive {
std::uint64_t Position = 0;
std::uint64_t Size = 0;
std::uint64_t Rotation = 0;
std::uint64_t Flags = 0;
std::uint64_t AssemblyLinearVelocity = 0;
}

namespace Model {
std::uint64_t PrimaryPart = 0;
}

namespace GuiObject {
std::uint64_t Position = 0;
std::uint64_t Size = 0;
std::uint64_t Visible = 0;
std::uint64_t Text = 0;
}

namespace UserInputService {
std::uint64_t WindowInputState = 0;
}

namespace WindowInputState {
std::uint64_t CurrentTextBox = 0;
}

namespace Lighting {
std::uint64_t FogEnd = 0;
}

namespace Atmosphere {
std::uint64_t Density = 0;
}

namespace AnimationTrack {
std::uint64_t Animation = 0;
std::uint64_t TimePosition = 0;
}

namespace Animator {
std::uint64_t ActiveAnimations = 0;
}
}

namespace FFlagOffsets {
namespace FFlags {
std::uint64_t NextGenReplicatorEnabledWrite4 = 0;
}
}

static std::string GetClientVersion(void) {
	HANDLE process = mem::get_process_handle(0);
	if (!process || process == INVALID_HANDLE_VALUE) return "";
	char path[MAX_PATH]{};
	if (!GetModuleFileNameExA(process, nullptr, path, MAX_PATH)) return "";
	return std::filesystem::path(path).parent_path().filename().string();
}

static void ParseOffsets(std::string& json_str) {
	static simdjson::ondemand::parser parser;
	simdjson::padded_string_view padded = simdjson::pad(json_str);
	simdjson::ondemand::document doc;
	(void)parser.iterate(padded).get(doc);
	simdjson::ondemand::object root;
	(void)doc.get_object().get(root);
	simdjson::ondemand::object offsets;
	(void)root["Offsets"].get_object().get(offsets);

	auto get = [&](const char* cat, const char* key, std::uint64_t& out) {
		(void)offsets[cat][key].get_uint64().get(out);
	};

	get("TaskScheduler", "Pointer", Offsets::TaskScheduler::Pointer);
	get("TaskScheduler", "MaxFPS", Offsets::TaskScheduler::MaxFPS);
	get("VisualEngine", "Pointer", Offsets::VisualEngine::Pointer);
	get("VisualEngine", "Dimensions", Offsets::VisualEngine::Dimensions);
	get("VisualEngine", "ViewMatrix", Offsets::VisualEngine::ViewMatrix);
	get("FakeDataModel", "Pointer", Offsets::FakeDataModel::Pointer);
	get("FakeDataModel", "RealDataModel", Offsets::FakeDataModel::RealDataModel);
	get("Instance", "Name", Offsets::Instance::Name);
	get("Instance", "ChildrenStart", Offsets::Instance::ChildrenStart);
	get("Instance", "ChildrenEnd", Offsets::Instance::ChildrenEnd);
	get("Instance", "Parent", Offsets::Instance::Parent);
	get("Instance", "ClassDescriptor", Offsets::Instance::ClassDescriptor);
	get("Instance", "ClassName", Offsets::Instance::ClassName);
	get("Instance", "AttributeContainer", Offsets::Instance::AttributeContainer);
	get("Instance", "AttributeList", Offsets::Instance::AttributeList);
	get("Instance", "AttributeToNext", Offsets::Instance::AttributeToNext);
	get("Instance", "AttributeToValue", Offsets::Instance::AttributeToValue);
	get("Misc", "Value", Offsets::Misc::Value);
	get("Misc", "AnimationId", Offsets::Misc::AnimationId);
	get("DataModel", "GameLoaded", Offsets::DataModel::GameLoaded);
	get("DataModel", "Workspace", Offsets::DataModel::Workspace);
	get("RunService", "HeartbeatTask", Offsets::RunService::HeartbeatTask);
	get("RunService", "HeartbeatFPS", Offsets::RunService::HeartbeatFPS);
	get("Workspace", "CurrentCamera", Offsets::Workspace::CurrentCamera);
	get("Player", "LocalPlayer", Offsets::Player::LocalPlayer);
	get("Player", "ModelInstance", Offsets::Player::ModelInstance);
	get("Humanoid", "Health", Offsets::Humanoid::Health);
	get("Humanoid", "MaxHealth", Offsets::Humanoid::MaxHealth);
	get("Humanoid", "MoveDirection", Offsets::Humanoid::MoveDirection);
	get("StatsItem", "Value", Offsets::StatsItem::Value);
	get("Camera", "Rotation", Offsets::Camera::Rotation);
	get("BasePart", "Primitive", Offsets::BasePart::Primitive);
	get("Primitive", "Position", Offsets::Primitive::Position);
	get("Primitive", "Size", Offsets::Primitive::Size);
	get("Primitive", "Rotation", Offsets::Primitive::Rotation);
	get("Primitive", "Flags", Offsets::Primitive::Flags);
	get("Primitive", "AssemblyLinearVelocity", Offsets::Primitive::AssemblyLinearVelocity);
	get("Model", "PrimaryPart", Offsets::Model::PrimaryPart);
	get("GuiObject", "Position", Offsets::GuiObject::Position);
	get("GuiObject", "Size", Offsets::GuiObject::Size);
	get("GuiObject", "Visible", Offsets::GuiObject::Visible);
	get("GuiObject", "Text", Offsets::GuiObject::Text);
	get("UserInputService", "WindowInputState", Offsets::UserInputService::WindowInputState);
	get("WindowInputState", "CurrentTextBox", Offsets::WindowInputState::CurrentTextBox);
	get("Lighting", "FogEnd", Offsets::Lighting::FogEnd);
	get("Atmosphere", "Density", Offsets::Atmosphere::Density);
	get("AnimationTrack", "Animation", Offsets::AnimationTrack::Animation);
	get("AnimationTrack", "TimePosition", Offsets::AnimationTrack::TimePosition);
	get("Animator", "ActiveAnimations", Offsets::Animator::ActiveAnimations);
}

static void ParseFFlags(std::string& json_str) {
	static simdjson::ondemand::parser parser;
	simdjson::padded_string_view padded = simdjson::pad(json_str);
	simdjson::ondemand::document doc;
	(void)parser.iterate(padded).get(doc);
	simdjson::ondemand::object root;
	(void)doc.get_object().get(root);
	simdjson::ondemand::object fflags;
	(void)root["FFlagOffsets"]["FFlags"].get_object().get(fflags);

	auto get = [&](const char* key, std::uint64_t& out) {
		(void)fflags[key].get_uint64().get(out);
	};
	get("NextGenReplicatorEnabledWrite4", FFlagOffsets::FFlags::NextGenReplicatorEnabledWrite4);
}

int Offsets::Fetched(void) {
	return offsets_initialized;
}

void Offsets::Reset(void) {
	offsets_initialized = false;
	Offsets::TaskScheduler::Pointer = 0;
	Offsets::TaskScheduler::MaxFPS = 0;
	Offsets::VisualEngine::Pointer = 0;
	Offsets::VisualEngine::Dimensions = 0;
	Offsets::VisualEngine::ViewMatrix = 0;
	Offsets::FakeDataModel::Pointer = 0;
	Offsets::FakeDataModel::RealDataModel = 0;
	Offsets::Instance::Name = 0;
	Offsets::Instance::ChildrenStart = 0;
	Offsets::Instance::ChildrenEnd = 0;
	Offsets::Instance::Parent = 0;
	Offsets::Instance::ClassDescriptor = 0;
	Offsets::Instance::ClassName = 0;
	Offsets::Instance::AttributeContainer = 0;
	Offsets::Instance::AttributeList = 0;
	Offsets::Instance::AttributeToNext = 0;
	Offsets::Instance::AttributeToValue = 0;
	Offsets::Misc::Value = 0;
	Offsets::Misc::AnimationId = 0;
	Offsets::DataModel::GameLoaded = 0;
	Offsets::DataModel::Workspace = 0;
	Offsets::RunService::HeartbeatTask = 0;
	Offsets::RunService::HeartbeatFPS = 0;
	Offsets::Workspace::CurrentCamera = 0;
	Offsets::Player::LocalPlayer = 0;
	Offsets::Player::ModelInstance = 0;
	Offsets::Humanoid::Health = 0;
	Offsets::Humanoid::MaxHealth = 0;
	Offsets::Humanoid::MoveDirection = 0;
	Offsets::StatsItem::Value = 0;
	Offsets::Camera::Rotation = 0;
	Offsets::BasePart::Primitive = 0;
	Offsets::Primitive::Position = 0;
	Offsets::Primitive::Size = 0;
	Offsets::Primitive::Rotation = 0;
	Offsets::Primitive::Flags = 0;
	Offsets::Primitive::AssemblyLinearVelocity = 0;
	Offsets::Model::PrimaryPart = 0;
	Offsets::GuiObject::Position = 0;
	Offsets::GuiObject::Size = 0;
	Offsets::GuiObject::Visible = 0;
	Offsets::GuiObject::Text = 0;
	Offsets::UserInputService::WindowInputState = 0;
	Offsets::WindowInputState::CurrentTextBox = 0;
	Offsets::Lighting::FogEnd = 0;
	Offsets::Atmosphere::Density = 0;
	Offsets::AnimationTrack::Animation = 0;
	Offsets::AnimationTrack::TimePosition = 0;
	Offsets::Animator::ActiveAnimations = 0;
	FFlagOffsets::FFlags::NextGenReplicatorEnabledWrite4 = 0;
}

void Offsets::Fetch(void) {
	if (offsets_initialized) return;
	std::string version = GetClientVersion();
	std::vector<http::request> requests = {
		{ "https://imtheo.lol/Offsets/" + version + "/Offsets.json", "", "", false },
		{ "https://imtheo.lol/Offsets/" + version + "/FFlags.json", "", "", false }
	};
	std::vector<http::result> results = http::curl_multi(requests);
	if (results.size() < 2) return;
	if (!(results[0].ok && !results[0].body.empty() && results[1].ok && !results[1].body.empty())) {
		MessageBoxA(nullptr, "ROBLOX VERSION MISMATCH!", "https://imtheo.lol/", MB_OK | MB_ICONWARNING | MB_TOPMOST | MB_SETFOREGROUND);
		ExitProcess(0);
	}
	ParseOffsets(results[0].body);
	offsets_initialized = true;
	ParseFFlags(results[1].body);
}

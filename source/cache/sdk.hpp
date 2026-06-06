#pragma once
#include <cstddef>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>
#include <windows.h>

extern std::uint64_t Character;
extern std::uint64_t Humanoid;
extern std::uint64_t Chat;
extern std::uint64_t Menu;

namespace Engine {
	struct Vector2 { float x{}, y{}; };
	struct Vector3 {
		float x{}, y{}, z{};
		Vector3 operator+(const Vector3& r) const { return { x + r.x, y + r.y, z + r.z }; }
		Vector3 operator-(const Vector3& r) const { return { x - r.x, y - r.y, z - r.z }; }
		Vector3 operator*(float s) const { return { x * s, y * s, z * s }; }
		float Dot(const Vector3& r) const { return x * r.x + y * r.y + z * r.z; }
		float Magnitude() const { float d = Dot(*this); return d > 0.f ? std::sqrt(d) : 0.f; }
		Vector3 Normalize() const { float l = Magnitude(); return l > 0.001f ? (*this) * (1.f / l) : Vector3{}; }
	};
	struct Vector4 { float x{}, y{}, z{}, w{}; };
	struct UDim {
		float Scale{};
		std::int32_t Offset{};
	};
	struct UDim2 {
		UDim X{};
		UDim Y{};
	};
	struct Matrix3x3 { float data[9]; };
	struct Matrix4x4 { float data[16]; };
	inline Vector3 operator*(const Matrix3x3& m, const Vector3& v) {
		return { m.data[0]*v.x + m.data[1]*v.y + m.data[2]*v.z,
			m.data[3]*v.x + m.data[4]*v.y + m.data[5]*v.z,
			m.data[6]*v.x + m.data[7]*v.y + m.data[8]*v.z };
	}
	inline Vector4 Multiply(const Matrix4x4& m, const Vector4& v) {
		return { m.data[0]*v.x + m.data[1]*v.y + m.data[2]*v.z + m.data[3]*v.w,
			m.data[4]*v.x + m.data[5]*v.y + m.data[6]*v.z + m.data[7]*v.w,
			m.data[8]*v.x + m.data[9]*v.y + m.data[10]*v.z + m.data[11]*v.w,
			m.data[12]*v.x + m.data[13]*v.y + m.data[14]*v.z + m.data[15]*v.w };
	}
	__forceinline Vector2 WorldToScreen(Vector3 world, Vector2 dims, Matrix4x4 view) {
		Vector4 clip = Multiply(view, Vector4{ world.x, world.y, world.z, 1.f });
		if (clip.w < 0.1f) return { -1.f, -1.f };
		float inv_w = 1.f / clip.w;
		return { (dims.x * 0.5f) * (clip.x * inv_w + 1.f), (dims.y * 0.5f) * (1.f - clip.y * inv_w) };
	}
}

struct WindowState { bool foreground; POINT position; RECT client_rect; };
struct UIActiveState { bool chat; bool menu; };
// inline HasEffect Offset{};

struct Instance {
	std::uint64_t address{};
	std::vector<Instance> GetChildren() const;
	Instance FindFirstChild(std::string name) const;
	Instance FindFirstChildOfClass(std::string name) const;
	std::string GetInstanceName() const;
	std::string GetInstanceClassName() const;
	std::string GetAttribute(const char* name) const;
};

struct SDK {
	static std::uint64_t ModuleBase;
	static Instance DataModel;
	static Instance Players;
	static Instance LocalPlayer;
	static Instance Character;
	static Instance Humanoid;
	static Instance HumanoidRootPart;
	static std::uint64_t HumanoidRootPartPrimitive;
	static Instance GroundSensor;
	static Instance SpellFrame;
	static Instance Symbols;
	static std::uint64_t CanCollide;
	static std::uint8_t CanCollideMask;
	static Instance Lighting;
	static std::uint64_t FogEnd;
	static std::vector<Instance> Atmosphere;
	static std::vector<float> Density;
	static std::uint64_t GameLoaded;
	static std::uint64_t HeartbeatFPS;
	static std::uint64_t Ping;
	static std::uint64_t TaskSchedulerMaxFPS;
	static float fly_speed;
	static bool fly_enabled;
	static bool no_fog;
	static bool hitbox_visual;
	static bool health_visual;
	static void Load(void);
	static void Unload(void);
	static void ReloadCharacter(void);
	static UIActiveState UIActive(void);
	static HWND WindowHandle;
	static WindowState Window(void);
};

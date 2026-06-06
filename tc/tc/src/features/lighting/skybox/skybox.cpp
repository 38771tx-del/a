#include "skybox.h"

#include <thread>
#include <chrono>

#include <memory/memory.h>
#include <sdk/offsets.h>
#include <sdk/sdk.h>
#include <game/game.h>
#include <settings.h>

namespace lighting
{
	namespace skybox
	{
		static rbx::instance_t m_cached_lighting{};
		static rbx::instance_t m_cached_sky{};
		static std::chrono::steady_clock::time_point m_last_lighting_check = std::chrono::steady_clock::now();
		static std::chrono::steady_clock::time_point m_last_invalidate = std::chrono::steady_clock::now();
		static uint64_t last_dm = 0;

		static std::string m_last_skybox_bk{};
		static std::string m_last_skybox_dn{};
		static std::string m_last_skybox_ft{};
		static std::string m_last_skybox_lf{};
		static std::string m_last_skybox_rt{};
		static std::string m_last_skybox_up{};

		static void skybox_thread()
		{
			while (true)
			{
				try
				{
					if (!settings::globals::is_game_active)
					{
						std::this_thread::sleep_for(std::chrono::milliseconds(200));
						continue;
					}

					if (game::datamodel.address == 0)
					{
						std::this_thread::sleep_for(std::chrono::milliseconds(500));
						continue;
					}

					if (game::datamodel.address != last_dm)
					{
						last_dm = game::datamodel.address;
						m_cached_lighting = {};
						m_cached_sky = {};
						std::this_thread::sleep_for(std::chrono::seconds(1));
						continue;
					}

					if (m_cached_lighting.address == 0 || std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - m_last_lighting_check).count() > 5)
					{
						m_cached_lighting = game::datamodel.find_first_child_by_class("Lighting");
						if (m_cached_lighting.address != 0)
						{
							m_cached_sky = m_cached_lighting.find_first_child_by_class("Sky");
						}
						m_last_lighting_check = std::chrono::steady_clock::now();
					}

					if (m_cached_lighting.address == 0)
					{
						std::this_thread::sleep_for(std::chrono::milliseconds(500));
						continue;
					}

					if (settings::lighting::skybox::enabled && m_cached_sky.address != 0)
					{
						auto sky = m_cached_sky;

						bool should_apply = settings::lighting::skybox::apply_skybox;

						if (should_apply)
						{
							try
							{
								if (sky.address == 0)
								{
									settings::lighting::skybox::apply_skybox = false;
									continue;
								}

								if (!settings::lighting::skybox::skybox_bk.empty())
								{
									memory->write_string(sky.address + Offsets::Sky::SkyboxBk, settings::lighting::skybox::skybox_bk);
									m_last_skybox_bk = settings::lighting::skybox::skybox_bk;
								}
								if (!settings::lighting::skybox::skybox_dn.empty())
								{
									memory->write_string(sky.address + Offsets::Sky::SkyboxDn, settings::lighting::skybox::skybox_dn);
									m_last_skybox_dn = settings::lighting::skybox::skybox_dn;
								}
								if (!settings::lighting::skybox::skybox_ft.empty())
								{
									memory->write_string(sky.address + Offsets::Sky::SkyboxFt, settings::lighting::skybox::skybox_ft);
									m_last_skybox_ft = settings::lighting::skybox::skybox_ft;
								}
								if (!settings::lighting::skybox::skybox_lf.empty())
								{
									memory->write_string(sky.address + Offsets::Sky::SkyboxLf, settings::lighting::skybox::skybox_lf);
									m_last_skybox_lf = settings::lighting::skybox::skybox_lf;
								}
								if (!settings::lighting::skybox::skybox_rt.empty())
								{
									memory->write_string(sky.address + Offsets::Sky::SkyboxRt, settings::lighting::skybox::skybox_rt);
									m_last_skybox_rt = settings::lighting::skybox::skybox_rt;
								}
								if (!settings::lighting::skybox::skybox_up.empty())
								{
									memory->write_string(sky.address + Offsets::Sky::SkyboxUp, settings::lighting::skybox::skybox_up);
									m_last_skybox_up = settings::lighting::skybox::skybox_up;
								}

								auto now = std::chrono::steady_clock::now();
								if (std::chrono::duration_cast<std::chrono::milliseconds>(now - m_last_invalidate).count() > 250)
								{
									uint64_t ptr1 = memory->read<uint64_t>(game::datamodel.address + 0x1D0);
									if (ptr1 != 0)
									{
										uint64_t ptr2 = memory->read<uint64_t>(ptr1 + 0x8);
										if (ptr2 != 0)
										{
											uint64_t renderView = memory->read<uint64_t>(ptr2 + 0x28);
											if (renderView != 0)
											{
												memory->write<bool>(renderView + Offsets::RenderView::SkyValid, false);
												memory->write<bool>(renderView + Offsets::RenderView::LightingValid, false);
											}
										}
									}
									m_last_invalidate = now;
								}
							}
							catch (...)
							{
							}

							settings::lighting::skybox::apply_skybox = false;
						}
					}

					if (settings::lighting::clocktime::enabled && m_cached_lighting.address != 0)
					{
						auto now = std::chrono::steady_clock::now();
						if (std::chrono::duration_cast<std::chrono::milliseconds>(now - m_last_invalidate).count() > 250)
						{
							uint64_t ptr1 = memory->read<uint64_t>(game::datamodel.address + 0x1D0);
							if (ptr1 != 0)
							{
								uint64_t ptr2 = memory->read<uint64_t>(ptr1 + 0x8);
								if (ptr2 != 0)
								{
									uint64_t renderView = memory->read<uint64_t>(ptr2 + 0x28);
									if (renderView != 0)
									{
										memory->write<bool>(renderView + Offsets::RenderView::LightingValid, false);
										memory->write<bool>(renderView + Offsets::RenderView::SkyValid, false);
									}
								}
							}
							m_last_invalidate = now;
						}
					}

					if (!settings::lighting::skybox::enabled)
					{
						m_last_skybox_bk.clear();
						m_last_skybox_dn.clear();
						m_last_skybox_ft.clear();
						m_last_skybox_lf.clear();
						m_last_skybox_rt.clear();
						m_last_skybox_up.clear();
					}
				}
				catch (...)
				{
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
		}

		void run()
		{
			static bool initialized = false;
			if (!initialized)
			{
				std::thread(skybox_thread).detach();
				initialized = true;
			}
		}
	}
}

idf_component_register(SRCS "am_map.c" "d_client.c" "d_items.c" "d_main.c" "f_finale.c" "f_wipe.c" 
					"g_game.c" "global_data.c" "hu_lib.c" "hu_stuff.c" "i_audio.c" "i_main.c" "info.c" 
					"i_system.c" "i_video.c" "lprintf.c" "lcd.c" "input.c" "m_bbox.c" "m_cheat.c" 
					"m_menu.c" "m_random.c" "m_recip.c" "p_ceilng.c" "p_doors.c" "p_enemy.c" 
					"p_floor.c" "p_genlin.c" "p_inter.c" "p_lights.c" "p_map.c" "p_maputl.c" 
					"p_mobj.c" "p_plats.c" "p_pspr.c" "p_setup.c" "p_sight.c" "p_spec.c" "p_switch.c" 
					"p_telept.c" "p_tick.c" "p_user.c" "r_data.c" "r_draw.c" "r_hotpath.iwram.c" 
					"r_main.c" "r_patch.c" "r_plane.c" "r_things.c" "sounds.c" "s_sound.c" "st_gfx.c" 
					"st_lib.c" "st_stuff.c" "tables.c" "version.c" "v_video.c" "wi_stuff.c" "w_wad.c" 
					"z_bmalloc.c" "z_zone.c" "snd_c3.c" "dbopl.c"
                    INCLUDE_DIRS ".")

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-error 
		-Wno-error=maybe-uninitialized -Wno-error=missing-field-initializers
		-Wno-error=uninitialized -Wno-error=unused-value -DHAVE_CONFIG_H -DHAVE_UNISTD_H
		-Wno-error=switch -Wno-error=unused-but-set-parameter -Wno-error=misleading-indentation
		-Wno-error=pointer-sign -Wno-error=parentheses)

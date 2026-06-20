cur_makefile := $(lastword $(MAKEFILE_LIST))

$(cur_makefile): ;

# Backward compatibility
asflags-y  += $(EXTRA_AFLAGS)
ccflags-y  += $(EXTRA_CFLAGS)
cppflags-y += $(EXTRA_CPPFLAGS)
ldflags-y  += $(EXTRA_LDFLAGS)

#
# flags that take effect in sub directories
export KBUILD_SUBDIR_ASFLAGS := $(KBUILD_SUBDIR_ASFLAGS) $(subdir-asflags-y)
export KBUILD_SUBDIR_CCFLAGS := $(KBUILD_SUBDIR_CCFLAGS) $(subdir-ccflags-y)

# Figure out what we need to build from the various variables
# ===========================================================================

real-objs-y		:=

# Handle archives
# ---------------------------------------------------------------------------

# Extract archives from $(obj-y)
archive-y				:= $(filter %.a, $(obj-y))
archive_custom_valid	:= $(filter $(archive_custom), $(archive-y))

ifeq ($(archive-y),)
archive-src-y	:=
archive-bin-y	:=
else # archive-y
ifeq ($(FORCE_TO_USE_LIB),1)
archive-bin-y	:= $(archive-y)
else
# Extract archives that have source files available
get-archive-src = $(wildcard $(addprefix $(srctree)/$(src)/,$($(1:.a=-y):.o=.*)))
archive-src-y	:= $(foreach m, $(archive-y), $(if $(call get-archive-src,$(m)), $(m)))

# Extract archives that do NOT have source files
archive-bin-y	:= $(filter-out $(archive-src-y), $(archive-y))
endif

# If the source files of the archive (including archive_custom_valid) exist, build the source files directly
archive-obj-y	:= $(foreach m, $(archive-src-y) $(archive_custom_valid), $($(m:.a=-y)))
real-objs-y		+= $(filter-out %/, $(archive-obj-y))
subdir-y		+= $(patsubst %/, %, $(filter %/, $(archive-obj-y)))
archive-objs-y	:= $(patsubst %/, %/built-in$(built_in_suffix), $(archive-objs-y))
endif # archive-y

# Handle composite objects
# ---------------------------------------------------------------------------

# if $(foo-objs) exists, foo.o is a composite object
multi-used-y := $(sort $(foreach m,$(obj-y), $(if $(strip $($(m:.o=-objs)) $($(m:.o=-y))), $(m))))

# Build list of the parts of our composite objects, our composite
# objects depend on those (obviously)
multi-objs-y		:= $(foreach m, $(multi-used-y), $($(m:.o=-objs)) $($(m:.o=-y)))
real-objs-y			+= $(filter-out %/ %.a, $(multi-obj-y))
multi-objs-subdir 	:= $(patsubst %/,%,$(filter %/, $(multi-objs-y)))
multi-objs-archive	:= $(filter %.a, $(multi-objs-y))
multi-objs-y		:= $(patsubst %/, %/built-in$(built_in_suffix), $(multi-objs-y))

define multi_subdir_built_in
$(foreach m, $(1), $(if $($(m:.o=$(strip $2))), $(eval $(m:.o=$(strip $2)) := $(patsubst %/, %/built-in$(built_in_suffix), $($(m:.o=$(strip $2)))))))
endef

ifneq ($(multi-objs-subdir),)
$(call multi_subdir_built_in,$(multi-used-y),-objs)
$(call multi_subdir_built_in,$(multi-used-y),-y)
subdir-y	+= $(multi-objs-subdir)
endif

ifneq ($(multi-objs-archive),)
ifneq ($(filter $(archive_custom), $(multi-objs-archive)),)
$(error Composite objects cannot be built from customized archives)
endif
mobjs-archive-src-y	:=
mobjs-archive-bin-y	:=
mobjs-archive-y		:= $(multi-objs-archive)

ifeq ($(FORCE_TO_USE_LIB),1)
mobjs-archive-bin-y	+= $(mobjs-archive-y)
else
# Extract archives that have source files available
mobjs-archive-src-y	+= $(foreach m, $(mobjs-archive-y), $(if $(wildcard $(addprefix $(srctree)/$(src)/,$($(m:.a=-y):.o=.*))), $(m)))

# Extract archives that do NOT have source files
mobjs-archive-bin-y	:= $(filter-out $(mobjs-archive-src-y), $(mobjs-archive-y))
endif

# If the source files of the archive (including archive_custom_valid) exist, build the source files directly
mobjs-obj-y			:= $(foreach m, $(mobjs-archive-src-y), $($(m:.a=-y)))

archive-y			+= $(mobjs-archive-y)
archive-bin-y		+= $(mobjs-archive-bin-y)
archive-src-y		+= $(mobjs-archive-src-y)
#obj-y				+= $(mobjs-obj-y)
subdir-y			+= $(patsubst %/,%,$(filter %/, $(mobjs-obj-y)))
real-objs-y			+= $(filter-out %/, $(mobjs-obj-y))
endif # multi-objs-archive

# Handle objects in subdirs
# ---------------------------------------------------------------------------
# o if we encounter foo/ in $(obj-y), replace it by foo/built-in.o or .a
#   and add the directory to the list of dirs to descend into: $(subdir-y)

__subdir-y	:= $(patsubst %/,%,$(filter %/, $(obj-y)))
subdir-y	+= $(__subdir-y)
obj-y		:= $(patsubst %/, %/built-in$(built_in_suffix), $(obj-y))

# Subdirectories we need to descend into
subdir-y	:= $(sort $(subdir-y))

# $(subdir-obj-y) is the list of objects in $(obj-y) which uses dir/ to
# tell kbuild to descend
subdir-obj-y := $(foreach m, $(subdir-y), $(m)/built-in$(built_in_suffix))

# $(obj-dirs) is a list of directories that contain object files
obj-dirs := $(dir $(archive-obj-y) $(multi-objs-y) $(obj-y))

# Replace multi-part objects by their individual parts, look at local dir only
real-objs-y += $(filter-out $(subdir-obj-y), $(obj-y)) $(extra-y)

# Add subdir path

extra-y		:= $(addprefix $(obj)/,$(extra-y))
always		:= $(addprefix $(obj)/,$(always))
targets		:= $(addprefix $(obj)/,$(targets))
obj-y		:= $(addprefix $(obj)/,$(obj-y))
lib-y		:= $(addprefix $(obj)/,$(lib-y))
subdir-obj-y	:= $(addprefix $(obj)/,$(subdir-obj-y))
real-objs-y	:= $(addprefix $(obj)/,$(real-objs-y))
multi-used-y	:= $(addprefix $(obj)/,$(multi-used-y))
multi-objs-y	:= $(addprefix $(obj)/,$(multi-objs-y))
subdir-y	:= $(addprefix $(obj)/,$(subdir-y))
obj-dirs	:= $(addprefix $(obj)/,$(obj-dirs))
archive-y	:= $(addprefix $(obj)/,$(archive-y))
archive-src-y	:= $(addprefix $(obj)/,$(archive-src-y))
archive-bin-y	:= $(addprefix $(obj)/,$(archive-bin-y))
archive_custom_valid	:= $(addprefix $(obj)/,$(archive_custom_valid))

orig_c_flags   = $(KBUILD_CPPFLAGS) $(KBUILD_CFLAGS) $(KBUILD_SUBDIR_CCFLAGS) \
                 $(ccflags-y) $(CFLAGS_$(basetarget).o)
_c_flags       = $(filter-out $(CFLAGS_REMOVE_$(basetarget).o), $(orig_c_flags) $(C_ONLY_FLAGS))
_c++_flags     = $(filter-out $(CFLAGS_REMOVE_$(basetarget).o), $(orig_c_flags) $(C++_ONLY_FLAGS))
_a_flags       = $(KBUILD_CPPFLAGS) $(KBUILD_AFLAGS) $(KBUILD_SUBDIR_ASFLAGS) \
                 $(asflags-y) $(AFLAGS_$(basetarget).o)
_cpp_flags     = $(KBUILD_CPPFLAGS) $(cppflags-y) $(CPPFLAGS_$(@F))

# If building the kernel in a separate objtree expand all occurrences
# of -Idir to -I$(srctree)/dir except for absolute paths (starting with '/').

ifeq ($(KBUILD_SRC),)
__c_flags	= $(_c_flags)
__c++_flags	= $(_c++_flags)
__a_flags	= $(_a_flags)
__cpp_flags     = $(_cpp_flags)
else

# -I$(obj) locates generated .h files
# $(call addtree,-I$(obj)) locates .h files in srctree, from generated .c files
#   and locates generated .h files
# FIXME: Replace both with specific CFLAGS* statements in the makefiles
__c_flags	= $(call addtree,-I$(obj)) $(call flags,_c_flags)
__c++_flags	= $(call addtree,-I$(obj)) $(call flags,_c++_flags)
__a_flags	=                          $(call a_flags_func,_a_flags)
__cpp_flags	=                          $(call flags,_cpp_flags)
endif

ifeq ($(WIN_PLAT),y)
# Not to include system header files in dependency files,
# for there are space characters in system path by default.
depfile_flags  = -MMD -MP -MF $(depfile) -MT $@
else
depfile_flags  = -MD -MP -MF $(depfile) -MT $@
endif

# Ordered de-duplication (keep first occurrence). mingw32-make 3.82 on Windows
# truncates recipe command lines past the ~8191 cmd.exe limit ("sh: line 2:
# syntax error"); the SDK's flag lists carry many duplicate -D/-I (e.g. -DIBRT_UI
# x3, the whole bthost include block twice), so de-duping brings commands under it.
uniqflags = $(if $1,$(firstword $1) $(call uniqflags,$(filter-out $(firstword $1),$1)))

c_flags        = $(call uniqflags,$(depfile_flags) $(NOSTDINC_FLAGS) $(__c_flags))

c++_flags      = $(call uniqflags,$(depfile_flags) $(NOSTDINC_FLAGS) $(__c++_flags))

a_flags        = $(call uniqflags,$(depfile_flags) $(NOSTDINC_FLAGS) $(__a_flags))

a_cpp_flags    = $(a_flags)

cpp_flags      = $(depfile_flags) $(NOSTDINC_FLAGS) $(__cpp_flags)

ld_flags       = $(LDFLAGS) $(ldflags-y)

# Useful for describing the dependency of composite objects
# Usage:
#   $(call multi_depend, multi_used_targets, suffix_to_remove, suffix_to_add)
define multi_depend
$(foreach m, $(notdir $1), \
	$(eval $(obj)/$m: \
	$(addprefix $(obj)/, $(foreach s, $3, $($(m:%$(strip $2)=%$(s)))))))
endef

# Commands useful for building a boot image
# ===========================================================================
#
#	Use as following:
#
#	target: source(s) FORCE
#		$(if_changed,ld/objcopy/gzip)
#
#	and add target to extra-y so that we know we have to
#	read in the saved command line

# Linking
# ---------------------------------------------------------------------------

quiet_cmd_ld = LD      $@
cmd_ld = $(LD) $(LDFLAGS) $(ldflags-y) $(LDFLAGS_$(@F)) \
	       $(filter-out FORCE,$^) -o $@

# Objcopy
# ---------------------------------------------------------------------------

quiet_cmd_objcopy = OBJCOPY $@
cmd_objcopy = $(OBJCOPY) $(OBJCOPYFLAGS) $(OBJCOPYFLAGS_$(@F)) $< $@



cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj_c += clear/
obj_c += watch/
obj_c += ui/


#obj_c += imgA8/
#obj_c += imgA4/

#obj_c +=  tiger/

obj_c +=  vector/
obj_c +=  path-gly/
obj_c +=  linearGrad/
#obj_c +=  cube_rotate/
obj_c +=  glyphs2/
#obj_c +=  glyphs/
#obj_c +=  Elementary/


obj_c += util/
obj_c += perf_test/

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ccflags-y := \



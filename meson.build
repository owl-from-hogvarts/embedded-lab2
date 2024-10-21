
# Took from here: https://github.com/hwengineer/STM32F3Discovery-meson-example/blob/master/meson.build
# Adapted for stm32f4

project('blink', 'c',
          default_options : ['b_lto=false',
                             'b_asneeded=false'])

#==============================================================================#
# Initialize some globals
srcs          = []
incdirs       = []
c_args        = []
link_args     = []
link_deps     = []

# compiler_version = meson.get_compiler('c').version()
# COMPILER_PATH = '/usr/lib/gcc/arm-none-eabi/' + compiler_version

#==============================================================================#
# Controller / HAL Driver dependend options
c_args     += '-DSTM32F427xx' # HAL Driver define
linkfiles   = files(['STM32F407VGTx_FLASH.ld'])
startupfile = files(['Startup/startup_stm32f407xx.s'])

#==============================================================================#
# convenience function : get correct -mcpu flag depending on hostmachine
cpu = host_machine.cpu() == 'cortex-m0+' ? 'cortex-m0plus' : host_machine.cpu()

#==============================================================================#
# convenience function : link correct C-runtime for each controller
# TODO: use llvm compiler-rt
arch = 'arm'
arch_version  = (host_machine.cpu() == 'cortex-m0') or (host_machine.cpu() == 'cortex-m0+') or (host_machine.cpu() == 'cortex-m1') ? 'v6-m'  : ''
arch_version += (host_machine.cpu() == 'cortex-m3') ?                                                                                'v7-m'  : ''
arch_version += (host_machine.cpu() == 'cortex-m4') or (host_machine.cpu() == 'cortex-m7') ?                                         'v7e-m' : ''

c_args += '-march=@0@'.format( arch + arch_version )

#==============================================================================#
# libc (newlib) include
# gcc links it but does not find headers
if build_machine.system() == 'linux'
  incdirs += include_directories('/usr/arm-none-eabi/include')
endif

#==============================================================================#
# convenience function : use correct Floating Point option
# TODO: as title says...

#==============================================================================#
# we have to define the std headers : for stdint.h
# if meson.get_compiler('c').get_id() == 'clang'
# incdirs += include_directories(join_paths(COMPILER_PATH, 'include')) # bare-metal : std header includes
# endif

#==============================================================================#
# create Debug Flags, depending on meson buildtype
c_args += get_option('buildtype') == 'release' ? '-DNDEBUG' : '-DDEBUG'

#==============================================================================#
# parse linkerfiles to a linker commando
foreach linkfile : linkfiles
 link_args += ['-Wl,-T,@0@'.format(join_paths(meson.current_source_dir(), linkfile.full_path()))]
endforeach

#==============================================================================#
# add STM library
subdir('Drivers')
# inlcude into project as lib requires config files from project
# include directories have to specify twice: once for build and once for actual import
# stm32f4_lib = static_library('stm32f4', stm32f4_src, include_directories: stm32f4_include)
# stm32f4 = declare_dependency(link_with: stm32f4_lib, include_directories: stm32f4_include)

# link_deps += stm32f4

srcs += stm32f4_src
incdirs += stm32f4_include

#==============================================================================#
# Sources

subdir('Core')

srcs += core_src
incdirs += core_include

#==============================================================================#
main = executable(
            'main',
            [srcs, startupfile],
            name_suffix         : 'elf',
            c_args              : c_args,
            link_args           : [link_args, '-Wl,--gc-sections'],
            dependencies        : link_deps,
            include_directories : incdirs )

#==============================================================================#
# import binary objects
objcopy  = '@0@'.format(find_program('objcopy').full_path())
objdump  = '@0@'.format(find_program('objdump').full_path())
size     = '@0@'.format(find_program('size').full_path())
# gdb      = '@0@'.format(find_program('gdb').full_path())
openocd  = '@0@'.format(find_program('openocd', required:false).full_path())

#==============================================================================#
# create custom target : create binary, hex dump, size and disassemble dump
mainbin = custom_target(
                        'main.bin',
    output           : ['main.bin'],
    # name_suffix      : 'bin',
    build_by_default : true,
    command          : [objcopy, '-O', 'binary', 'main.elf', 'main.bin'],
    depends          : [main])

mainhex = custom_target(
                            'main.hex',
        output           : ['main.hex'],
        build_by_default : true,
        command          : [objcopy, '-O', 'ihex', 'main.elf', 'main.hex'],
        depends          : [main])

mainsize = custom_target(
                          'size',
        capture          : true,
        output           : ['main.size'],
        build_by_default : true,
        command          : [size, '--format=berkeley', 'main.elf'],
        depends          : [main])

if meson.get_compiler('c').get_id() == 'clang'
    maindump = custom_target(
                          'dump',
        capture          : true,
        output           : 'main.dump',
        build_by_default : false,
        command          : [objdump, '-triple=@0@-none-eabi'.format(arch), '-disassemble-all', '-S', '-t', 'main.elf'],
        depends          : [main])

elif meson.get_compiler('c').get_id() == 'gcc'
  maindump = custom_target(
                        'dump',
      capture          : true,
      output           : 'main.dump',
      build_by_default : false,
      command          : [objdump, '-D', '-S', '-t', 'main.elf'],
      depends          : [main])
endif


#==============================================================================#
# support target commands
# run_target('gdb',
#          command : [terminal, '--working-directory=@0@'.format(meson.source_root()), '-e', gdb, '-q', '@0@/main.elf'.format(meson.current_build_dir())])

# run_target('openocd',
        #  command : [terminal, '-e', openocd, '-f', 'interface/stlink-v2.cfg', '-f' , 'target/stm32f3x.cfg'])

#==============================================================================#

# Debug commands
# message(host_machine.system())
# message(host_machine.cpu_family())
# message(host_machine.cpu())
# message(host_machine.endian())

# #==============================================================================#
# # include newlib_nano : but without C++ features
# # does not work at the moment...
# if meson.get_cross_property('newlib_nano') == true
#   message('newlib_nano added')
#   linkDeps +=  meson.get_compiler('c').find_library('/usr/lib/arm-none-eabi/newlib/libc_nano')
#   linkDeps +=  meson.get_compiler('c').find_library('/usr/lib/arm-none-eabi/newlib/librdimon_nano')
#   linkDeps +=  meson.get_compiler('c').find_library('/usr/lib/arm-none-eabi/newlib/libg_nano')
#   linkDeps +=  meson.get_compiler('c').find_library('/usr/lib/arm-none-eabi/newlib/libstdc++_nano')
#   linkDeps +=  meson.get_compiler('c').find_library('/usr/lib/arm-none-eabi/newlib/libsupc++_nano')
#
#   linkDeps +=  meson.get_compiler('c').find_library('/usr/lib/arm-none-eabi/newlib/libm')
#   # linkDeps +=  meson.get_compiler('c').find_library('/usr/lib/arm-none-eabi/newlib/libnosys')
# endif
#

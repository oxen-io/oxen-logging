local default_deps_base = [
  'libfmt-dev',
  'libspdlog-dev',
];

local default_deps = ['g++'] + default_deps_base;
local docker_base = 'registry.oxen.rocks/';

local submodule_commands = [
  'git fetch --tags',
  'git submodule update --init --recursive --depth=1 --jobs=4',
];
local submodules = {
  name: 'submodules',
  image: 'drone/git',
  commands: submodule_commands,
};

// cmake options for static deps mirror
local ci_dep_mirror(want_mirror) = (if want_mirror then ' -DLOCAL_MIRROR=https://oxen.rocks/deps ' else '');

local apt_get_quiet = 'apt-get -o=Dpkg::Use-Pty=0 -q';

local kitware_repo(distro) = [
  'eatmydata ' + apt_get_quiet + ' install -y curl ca-certificates',
  'curl -sSL https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - >/usr/share/keyrings/kitware-archive-keyring.gpg',
  'echo "deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ ' + distro + ' main" >/etc/apt/sources.list.d/kitware.list',
  'eatmydata ' + apt_get_quiet + ' update',
];

local debian_backports(distro, pkgs) = [
  'echo deb http://deb.debian.org/debian ' + distro + '-backports main >>/etc/apt/sources.list.d/backports.list',
  'eatmydata ' + apt_get_quiet + ' update',
  'eatmydata ' + apt_get_quiet + ' install -y ' + std.join(' ', std.map(function(p) p + '/' + distro + '-backports', pkgs)),
];

local generic_build(jobs, build_type, lto, werror, cmake_extra, local_mirror, tests, gdb=true)
      = [
          'mkdir build',
          'cd build',
          'cmake .. -DCMAKE_COLOR_DIAGNOSTICS=ON -DCMAKE_BUILD_TYPE=' + build_type + ' ' +
          (if werror then '-DOXEN_LOGGING_WARNINGS_AS_ERRORS=ON ' else '') +
          '-DWITH_LTO=' + (if lto then 'ON ' else 'OFF ') +
          cmake_extra +
          ci_dep_mirror(local_mirror),
          'make -j' + jobs + ' VERBOSE=1',
          'cd ..',
        ]
        + (if tests then [
             'cd build',
             (if gdb then '../utils/ci/drone-gdb.sh ' else '') + './tests/alltests --success --log-level debug --no-ipv6 --colour-mode ansi',
             'cd ..',
           ] else []);

// Regular build on a debian-like system:
local debian_pipeline(name,
                      image,
                      arch='amd64',
                      deps=default_deps,
                      extra_setup=[],
                      build_type='Release',
                      lto=false,
                      werror=true,
                      cmake_extra='',
                      local_mirror=true,
                      extra_cmds=[],
                      jobs=6,
                      tests=false,
                      oxen_repo=false,
                      allow_fail=false) = {
  kind: 'pipeline',
  type: 'docker',
  name: name,
  platform: { arch: arch },
  steps: [
    submodules,
    {
      name: 'build',
      image: image,
      pull: 'always',
      [if allow_fail then 'failure']: 'ignore',
      environment: { SSH_KEY: { from_secret: 'SSH_KEY' } },
      commands: [
                  'echo "Building on ${DRONE_STAGE_MACHINE}"',
                  'echo "man-db man-db/auto-update boolean false" | debconf-set-selections',
                  apt_get_quiet + ' update',
                  apt_get_quiet + ' install -y eatmydata',
                ] + (
                  if oxen_repo then [
                    'eatmydata ' + apt_get_quiet + ' install --no-install-recommends -y lsb-release',
                    'cp utils/deb.oxen.io.gpg /etc/apt/trusted.gpg.d',
                    'echo deb http://deb.oxen.io $$(lsb_release -sc) main >/etc/apt/sources.list.d/oxen.list',
                    'eatmydata ' + apt_get_quiet + ' update',
                  ] else []
                ) + extra_setup
                + [
                  'eatmydata ' + apt_get_quiet + ' dist-upgrade -y',
                  'eatmydata ' + apt_get_quiet + ' install --no-install-recommends -y cmake git pkg-config ccache ' + std.join(' ', deps),
                ]
                + generic_build(jobs, build_type, lto, werror, cmake_extra, local_mirror, tests)
                + extra_cmds,
    },
  ],
};
// windows cross compile on debian
local windows_cross_pipeline(name,
                             image,
                             arch='amd64',
                             build_type='Release',
                             lto=false,
                             werror=false,
                             cmake_extra='',
                             local_mirror=true,
                             cmake_extra='',
                             extra_cmds=[],
                             jobs=6,
                             tests=false,
                             allow_fail=false) = {
  kind: 'pipeline',
  type: 'docker',
  name: name,
  platform: { arch: arch },
  steps: [
    submodules,
    {
      name: 'build',
      image: image,
      pull: 'always',
      [if allow_fail then 'failure']: 'ignore',
      environment: { SSH_KEY: { from_secret: 'SSH_KEY' }, WINDOWS_BUILD_NAME: 'x64' },
      commands: [
        'echo "Building on ${DRONE_STAGE_MACHINE}"',
        'echo "man-db man-db/auto-update boolean false" | debconf-set-selections',
        apt_get_quiet + ' update',
        apt_get_quiet + ' install -y eatmydata',
        'eatmydata ' + apt_get_quiet + ' install --no-install-recommends -y build-essential cmake git pkg-config ccache g++-mingw-w64-x86-64-posix',
        'mkdir build',
        'cd build',
        'cmake .. -DCMAKE_TOOLCHAIN_FILE=../utils/cross/mingw-x64.cmake -DBUILD_STATIC_DEPS=ON ' +
        '-DCMAKE_COLOR_DIAGNOSTICS=ON -DCMAKE_BUILD_TYPE=' + build_type + ' ' +
        (if werror then '-DOXEN_LOGGING_WARNINGS_AS_ERRORS=ON ' else '') +
        '-DWITH_LTO=' + (if lto then 'ON ' else 'OFF ') +
        ci_dep_mirror(local_mirror) +
        cmake_extra,
        'make -j' + jobs + ' VERBOSE=1',
        //'wine-stable tests/alltests.exe --log-level debug --colour-mode ansi', // doesn't work yet :(
      ] + extra_cmds,
    },
  ],
};

// linux cross compile on debian
local linux_cross_pipeline(name,
                           cross_targets,
                           arch='amd64',
                           build_type='Release',
                           cmake_extra='',
                           local_mirror=true,
                           extra_cmds=[],
                           jobs=6,
                           tests=false,
                           allow_fail=false) = {
  kind: 'pipeline',
  type: 'docker',
  name: name,
  platform: { arch: arch },
  steps: [
    submodules,
    {
      name: 'build',
      image: docker_base + 'debian-stable-cross',
      pull: 'always',
      [if allow_fail then 'failure']: 'ignore',
      environment: { SSH_KEY: { from_secret: 'SSH_KEY' }, CROSS_TARGETS: std.join(':', cross_targets) },
      commands: [
        'echo "Building on ${DRONE_STAGE_MACHINE}"',
        'VERBOSE=1 JOBS=' + jobs + ' ./contrib/cross.sh ' + std.join(' ', cross_targets) +
        ' -- ' + cmake_extra + ci_dep_mirror(local_mirror),
      ],
    },
  ],
};

local clang(version) = debian_pipeline(
  'Debian sid/clang-' + version + ' (amd64)',
  docker_base + 'debian-sid-clang',
  deps=['clang-' + version] + default_deps_base,
  cmake_extra='-DCMAKE_C_COMPILER=clang-' + version + ' -DCMAKE_CXX_COMPILER=clang++-' + version + (
    if version >= 21 then ' -DOXEN_LOGGING_FORCE_SUBMODULES=ON' else ' '
  )
);

local full_llvm(version) = debian_pipeline(
  'Debian sid/llvm-' + version + ' (amd64)',
  docker_base + 'debian-sid-clang',
  deps=['clang-' + version, ' lld-' + version, ' libc++-' + version + '-dev', 'libc++abi-' + version + '-dev']
       + default_deps_base,
  cmake_extra='-DCMAKE_C_COMPILER=clang-' + version +
              ' -DCMAKE_CXX_COMPILER=clang++-' + version +
              ' -DCMAKE_CXX_FLAGS=-stdlib=libc++ ' +
              std.join(' ', [
                '-DCMAKE_' + type + '_LINKER_FLAGS=-fuse-ld=lld-' + version
                for type in ['EXE', 'MODULE', 'SHARED']
              ]) +
              ' -DOXEN_LOGGING_FORCE_SUBMODULES=ON'
);

// Macos build
local mac_builder(name,
                  build_type='Release',
                  arch='amd64',
                  werror=true,
                  lto=false,
                  cmake_extra='',
                  local_mirror=true,
                  extra_cmds=[],
                  jobs=6,
                  tests=false,
                  allow_fail=false) = {
  kind: 'pipeline',
  type: 'exec',
  name: name,
  platform: { os: 'darwin', arch: arch },
  steps: [
    { name: 'submodules', commands: submodule_commands },
    {
      name: 'build',
      environment: { SSH_KEY: { from_secret: 'SSH_KEY' } },
      commands: [
                  'echo "Building on ${DRONE_STAGE_MACHINE}"',
                  // If you don't do this then the C compiler doesn't have an include path containing
                  // basic system headers.  WTF apple:
                  'export SDKROOT="$(xcrun --sdk macosx --show-sdk-path)"',
                ]
                + generic_build(jobs, build_type, lto, werror, cmake_extra, local_mirror, tests, gdb=false)
                + extra_cmds,
    },
  ],
};


[
  {
    name: 'lint check',
    kind: 'pipeline',
    type: 'docker',
    steps: [{
      name: 'build',
      image: docker_base + 'lint',
      pull: 'always',
      commands: [
        'echo "Building on ${DRONE_STAGE_MACHINE}"',
        apt_get_quiet + ' update',
        apt_get_quiet + ' install -y eatmydata',
        'eatmydata ' + apt_get_quiet + ' install --no-install-recommends -y git clang-format-16 jsonnet',
        './utils/ci/lint-check.sh',
      ],
    }],
  },

  // Various debian builds
  debian_pipeline('Debian sid (amd64)', docker_base + 'debian-sid'),
  debian_pipeline('Debian sid/Debug (amd64)', docker_base + 'debian-sid', build_type='Debug'),
  clang(19),
  full_llvm(19),
  clang(21),
  full_llvm(21),
  debian_pipeline('Debian sid', docker_base + 'debian-sid'),
  debian_pipeline('Debian sid/Debug', docker_base + 'debian-sid', build_type='Debug'),
  debian_pipeline('Debian 13 trixie', docker_base + 'debian-trixie'),
  debian_pipeline('Debian 13/Debug', docker_base + 'debian-trixie', build_type='Debug'),
  debian_pipeline('Debian testing (i386)', docker_base + 'debian-testing/i386'),
  debian_pipeline('Debian 12 bookworm (i386)', docker_base + 'debian-bookworm/i386'),
  debian_pipeline('Ubuntu latest', docker_base + 'ubuntu-rolling'),
  debian_pipeline('Ubuntu 24.04 noble', docker_base + 'ubuntu-noble'),
  debian_pipeline('Ubuntu 22.04 jammy', docker_base + 'ubuntu-jammy'),

  // ARM builds (ARM64 and armhf)
  debian_pipeline('Debian sid (ARM64)', docker_base + 'debian-sid', arch='arm64', jobs=4),
  debian_pipeline('Debian stable/Debug (ARM64)', docker_base + 'debian-stable', arch='arm64', jobs=4, build_type='Debug'),
  debian_pipeline('Debian stable (armhf)', docker_base + 'debian-stable/arm32v7', arch='arm64', jobs=4),

  // Windows builds (x64)
  windows_cross_pipeline('Windows (amd64)', docker_base + 'debian-win32-cross'),

  // Macos builds:
  mac_builder('macOS Intel (Release)'),
  mac_builder('macOS Arm64 (Release)', arch='arm64'),
  mac_builder('macOS Arm64 (Debug)', build_type='Debug', arch='arm64'),
]

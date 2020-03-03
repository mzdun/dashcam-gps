def cmakeOpts = '-DMGPS_BUILD_70MAI=ON -DMGPS_BUILD_QT5=ON -DMGPS_BUILD_TOOLS=ON -DMGPS_BUILD_TESTS=ON'
def cmakeOpts_rel = ' -DMGPS_PACK_COMPONENTS=OFF'
def builds = [
    [name: 'Release',  args: cmakeOpts, type: 'release', releaseable: true],
    [name: 'Debug',  args: cmakeOpts, type: 'debug'],
    [name: 'Fuzz-ready',  args: '-DMGPS_BUILD_70MAI=ON -DMGPS_BUILD_FUZZ=ON', type: 'fuzzy', releaseable: true],
    [name: 'Tests',  args: cmakeOpts + ' -DMGPS_BUILD_TESTS=ON -DMGPS_BUILD_TESTS_ONLY=ON', type: 'tests', basedOn: 'release', releaseable: true,
        allowNormalBuilds: false, environment: ["PROJECT_VERSION_BUILD_RELEASE_TESTS=1"]],
]

Map posix = [
    call: { String script, String label -> sh script:script, label:label },
    callNoCheck: { String script, String label -> sh script:script, label:label, returnStatus:true },
    env: { sh "printenv" },
    builds: [
        release: [ build: 'Release', generator: 'Ninja', packer: 'TGZ',
            steps: [
                [ args: 'all' ]
            ]
        ],
        debug: [ build: 'Debug', generator: 'Ninja', packer: 'TGZ',
            steps: [
                [ args: 'all' ]
            ]
        ]
    ]
]

Map linux = posix + [:]
linux.builds = linux.builds + [
    fuzzy: [ build: 'Release', generator: 'Ninja', packer: 'TGZ',
            steps: [
                [ args: 'all' ]
            ],
            environment: [ "CC=afl-gcc", "CXX=afl-g++" ]
        ],
]

Map windows = [
    call: { String script, String label -> bat script:"@${script}", label:label },
    callNoCheck: { String script, String label -> bat script:"@${script}", label:label, returnStatus:true },
    env: { bat "set" },
    builds: [
        release: [
            packer: 'ZIP',
            steps: [
                [ args: '-nologo -m -v:m -p:Configuration=Release' ]
            ],
            conan: [ 'compiler.runtime': 'MD' ],
            test_config: 'Release'
        ],
        debug: [
            packer: 'ZIP',
            steps: [
                [ args: '-nologo -m -v:m -p:Configuration=Debug' ]
            ],
            conan: [ 'compiler.runtime': 'MDd' ],
            test_config: 'Debug'
        ]
    ]
]

Map win32 = [
    call: windows.call,
    env: windows.env,
    callNoCheck: windows.callNoCheck,
    arch: 'x86',
    builds: [
        release: windows.builds.release + [
            arch: 'Win32',
            conan: windows.builds.release.conan + [ arch_target: 'x86', arch: 'x86' ]
        ],
        debug: windows.builds.debug + [
            arch: 'Win32',
            conan: windows.builds.debug.conan + [ arch_target: 'x86', arch: 'x86' ]
        ]
    ]
]

Map conan = [
    release: [build_type: 'Release'],
    fuzzy: [build_type: 'Release'],
    debug: [build_type: 'Debug']
]

def platforms = [
    [name: 'Linux', node: 'linux', os: linux ],
    [name: 'Win64', node: 'windows', os: windows ],
    [name: 'Win32', node: 'windows', os: win32 ],
    [name: 'MacOS', node: 'mac', os: posix ]
]

def createCMakeBuild(Map conan, Map os, Map task) {
    Map type = os.builds[task.type]
    if (type == null && task.containsKey('basedOn'))
        type = os.builds[task.basedOn]

    List environment = (os.environment ?: []) + (type.environment ?: []) + (task.environment ?: [])

    String cmakeBinary = env.CMAKE_BINARY ?: "cmake"
    String conanBinary = env.CONAN_BINARY ?: "conan"

    String cmakeConfigure = "${cmakeBinary}"

    if (type.containsKey('generator')) {
        cmakeConfigure += " -G \"${type.generator}\""
    }

    if (type.containsKey('arch')) {
        cmakeConfigure += " -A \"${type.arch}\""
    }

    if (type.containsKey('build')) {
        cmakeConfigure += " -DCMAKE_BUILD_TYPE=" + type.build
    }

    if (task.containsKey('args')) {
        cmakeConfigure += " ${task.args}";
    }

    cmakeConfigure += " ../..";

    String conanConfigure = ""
    Map conanArgs = conan + (type.conan ?: [:])
    conanArgs.each { conanConfigure += " -s $it.key=$it.value" }

    withEnv(environment) {
        os.call("${conanBinary} install ../.. --build missing${conanConfigure}", 'Get dependencies')
        os.call(cmakeConfigure, 'Generate build from CMake')


        if (type.containsKey('steps')) {
            String buildArgs = "${cmakeBinary} --build ."
            for (step in type.steps) {
                String stepArgs = "${buildArgs}"
                if (step.containsKey('args')) {
                    stepArgs += ' -- '
                    stepArgs += step.args
                }
                withEnv(step.env ?: []) {
                    os.call(stepArgs, null)
                }
            }
        }

        os.call("cpack -G ${type.packer}")

        if (cmakeConfigure.contains(' -DMGPS_BUILD_TESTS=ON ')) {
            String test_config = type.test_config ?: type.build
            if (test_config != null)
                test_config = " -C $test_config"
            else
                test_config = ''

            os.callNoCheck("ctest$test_config --output-on-failure")
        }
    }
}

def createJob(Map platform, Map build, Map conan) {
    Map os = platform.os
    String nodeLabel = platform.node
    String stageName = "${build.name}/${platform.name}"
    String arch = os.arch ?: 'x86_64'
    return {
        stage("${stageName}") {
            node("${nodeLabel}") {
                echo "[${env.NODE_NAME}] ${WORKSPACE}"
                checkout scm

                dir("build/${build.type}") {
                    List vars = []
                    if (arch == 'x86') {
                        vars = ["Qt5_DIR=${env.Qt5_DIR_x86}"]
                    }

                    withEnv(vars) {
                        def conanCfg = conan[build.type]
                            ?: (build.containsKey('basedOn') ? (conan[build.basedOn] ?: [:]) : [:])
                        createCMakeBuild(conanCfg, os, build)
                    }
                }

                def config = " ${build.args} "
                if (config.contains(' -DMGPS_BUILD_TESTS=ON ')) {
                    junit "build/${build.type}/testing-results/*.xml"
                }

                archiveArtifacts "build/${build.type}/dashcam-gps-*"
            }
        }
    }
}
properties([
    parameters([
        booleanParam(name: 'MAKE_A_RELEASE_BUILD', defaultValue: false,
            description: '''When turned on, will:
  a. change the build name from "#123" to "v2.71 [123]";
  b. generate a build with no SemVer build metadata;
  c. run only Release jobs (no Debug artifacts).

When turned off, will
  a. generate a build with +dev.mmddHHMMSS metadata''')
    ])
])

if (params.MAKE_A_RELEASE_BUILD) {
    stage('SemVer (Release)') {
        node("linux") {
            script {
                checkout scm
                sh "git fetch --tags"

                env.PROJECT_VERSION_BUILD_RELEASE = sh(script:"date +%Y%m%d", returnStdout: true).trim()
                String TAG = sh(script: "git describe --always", returnStdout: true).trim()
                String SHA = sh(script: "git rev-parse --short HEAD", returnStdout: true).trim()
                if (TAG == SHA || TAG.endsWith("-g$SHA"))
                    TAG = "git:$TAG"
                else if (TAG.take(1) != 'v')
                    TAG = "v$TAG"
                currentBuild.displayName = "$TAG [$currentBuild.number]"
            }
        }
    }
} else {
    stage('SemVer+meta') {
        node("linux") {
            script {
                env.PROJECT_VERSION_BUILD_META = sh(script:"date ++dev.%m%d%H%M%S", returnStdout: true).trim()
            }
        }
    }
}

stage('Build') {
    Map tasks = [:]
    for(platform in platforms) {
        for(build in builds) {
            if (!platform.os.builds.containsKey(build.type)) {
                if (!build.containsKey('basedOn')) {
                    continue
                }

                if (!platform.os.builds.containsKey(build.basedOn)) {
                    continue
                }
            }
            if (build.containsKey('filter') && !(platform.node in build.filter))
                continue

            def releaseable = build.containsKey('releaseable') ? build.releaseable : false
            def allowNormalBuilds = build.containsKey('allowNormalBuilds') ? build.allowNormalBuilds : true

            def should_build = releaseable == allowNormalBuilds ? allowNormalBuilds : allowNormalBuilds != params.MAKE_A_RELEASE_BUILD

            if (should_build) {
                def build_opts = build
                if (params.MAKE_A_RELEASE_BUILD) {
                    def args = build.args + cmakeOpts_rel
                    if (allowNormalBuilds) {
                        // Remove the testing from Release, it will return in Tests
                        args = " $args ".replaceAll(' -DMGPS_BUILD_TESTS=ON ', ' ')
                        args = args[1..(args.size()-2)]
                    }
                    build_opts = build + [args: args]
                }
                tasks["${build.name}/${platform.name}"] = createJob(platform, build_opts, conan)
            }
        }
    }

    parallel tasks
}

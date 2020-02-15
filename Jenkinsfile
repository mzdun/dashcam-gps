def builds = [
    [name: 'Release',  args: '-DMGPS_BUILD_70MAI=ON -DMGPS_BUILD_QT5=ON -DMGPS_BUILD_TOOLS=ON', type: 'release'],
    [name: 'Debug',  args: '-DMGPS_BUILD_70MAI=ON -DMGPS_BUILD_QT5=ON -DMGPS_BUILD_TOOLS=ON', type: 'debug'],
]

Map posix = [
    call: { String script, String label -> sh script:script, label:label },
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

Map windows = [
    call: { String script, String label -> bat script:"@${script}", label:label },
    env: { bat "set" },
    builds: [
        release: [
            packer: 'ZIP',
            steps: [
                [ args: '-nologo -m -v:m -p:Configuration=Release' ]
            ],
            conan: [ 'compiler.runtime': 'MD' ]
        ],
        debug: [
            packer: 'ZIP',
            steps: [
                [ args: '-nologo -m -v:m -p:Configuration=Debug' ]
            ],
            conan: [ 'compiler.runtime': 'MDd' ]
        ]
    ]
]

Map win32 = [
    call: windows.call,
    env: windows.env,
    arch: 'x86',
    builds: [
        release: windows.builds.release + [
            arch: 'Win32',
            conan: windows.builds.release.conan + [ arch_target: 'x86' ]
        ],
        debug: windows.builds.debug + [
            arch: 'Win32',
            conan: windows.builds.debug.conan + [ arch_target: 'x86' ]
        ]
    ]
]

Map conan = [
    release: [build_type: 'Release'],
    debug: [build_type: 'Debug']
]

def platforms = [
    [name: 'Linux', node: 'linux', os: posix ],
    [name: 'Win64', node: 'windows', os: windows ],
    [name: 'Win32', node: 'windows', os: win32 ],
    [name: 'MacOS', node: 'mac', os: posix ]
]

def createCMakeBuild(Map conan, Map os, Map task) {
    if (!os.builds.containsKey(task.type))
        return

    Map type = os.builds[task.type]

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
            if (step.containsKey('env')) {
                withEnv(step.env) {
                    os.call(stepArgs, null)
                }
            } else {
                os.call(stepArgs, null)
            }
        }
    }

    os.call("cpack -G ${type.packer}")
}

def createJob(Map platform, Map build, Map conan) {
    Map os = platform.os
    String nodeLabel = platform.node
    String stageName = "${build.name} (${platform.name})"
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
                        createCMakeBuild(conan[build.type] ?: [:], os, build)
                    }
                }

                archiveArtifacts "build/${build.type}/dashcam-gps-*"
            }
        }
    }
}

stage('SemVer+meta') {
    node("linux") {
        script {
            env.PROJECT_VERSION_BUILD_META = sh(script:"date ++dev.%m%d%H%M%S", returnStdout: true).trim()
        }
    }
}

stage('Build') {
    Map tasks = [:]
    for(platform in platforms) {
        for(build in builds) {
            if (build.containsKey('filter') && !(platform.node in build.filter))
                continue
            tasks["${build.name}/${platform.name}"] = createJob(platform, build, conan)
        }
    }

    parallel tasks
}

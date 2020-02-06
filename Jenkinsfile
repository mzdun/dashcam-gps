def builds = [
    [name: 'Qt5',   args: '-DMGPS_BUILD_70MAI=ON -DMGPS_BUILD_QT5=ON',   archive: 'dashcam-gps-qt5-player', type: 'release'],
    [name: 'Tools', args: '-DMGPS_BUILD_70MAI=ON -DMGPS_BUILD_TOOLS=ON', archive: 'dashcam-gps-tools',      type: 'release']
]

Map posix = [
    call: { String script, String label -> sh script:script, label:label },
    builds: [
        release: [ build: 'Release', generator: 'Ninja',
            steps: [
                [ args: 'all' ], [ args: 'install', env: ['DESTDIR=../artifacts'] ]
            ]
        ]
    ]
]

Map windows = [
    call: { String script, String label -> bat script:"@${script}", label:label },
    builds: [
        release: [
            steps: [
                [ args: '-nologo -m -v:m -p:Configuration=Release' ]
            ]
        ]
    ]
]

def platforms = [
    [name: 'Linux', node: 'linux', os: posix ],
    [name: 'Win64', node: 'windows', os: windows ],
    [name: 'MacOS', node: 'mac', os: posix ]
]

def createCMakeBuild(Map os, Map task) {
    if (!os.builds.containsKey(task.type))
        return

    Map type = os.builds[task.type]

    String cmakeBinary = env.CMAKE_BINARY ?: "cmake"
    String conanBinary = env.CONAN_BINARY ?: "conan"

    String cmakeConfigure = "${cmakeBinary}"

    if (type.containsKey('generator')) {
        cmakeConfigure += " -G \"${type.generator}\""
    }
    if (type.containsKey('build')) {
        cmakeConfigure += " -DCMAKE_BUILD_TYPE=" + type.build
    }

    if (task.containsKey('args')) {
        cmakeConfigure += " ${task.args}";
    }

    cmakeConfigure += " ..";

    os.call("${conanBinary} install .. --build missing", 'Get dependencies')
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
}

def createJob(Map platform, Map build) {
    Map os = platform.os
    String nodeLabel = platform.node
    String stageName = "${build.name} (${platform.name})"
    return {
        stage("${stageName}") {
            node("${nodeLabel}") {
                echo "[${env.NODE_NAME}] ${WORKSPACE}"
                checkout scm

                dir('build') {
                    createCMakeBuild(os, build)
                }

                os.call("python3 tools/pack.py ${build.archive}", 'Pack archives')
                archiveArtifacts "artifacts/${build.archive}-*"
            }
        }
    }
}

stage('Build') {
    Map tasks = [:]
    for(platform in platforms) {
        for(build in builds) {
            if (build.containsKey('filter') && !(platform.node in build.filter))
                continue
            tasks["${build.name}/${platform.name}"] = createJob(platform, build)
        }
    }

    parallel tasks
}

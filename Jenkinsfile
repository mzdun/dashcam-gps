def builds = [
    [name: 'Qt5',   args: '-DMGPS_BUILD_70MAI=ON -DMGPS_BUILD_QT5=ON',   archive: 'dashcam-gps-qt5-player', type: 'release'],
    [name: 'Tools', args: '-DMGPS_BUILD_70MAI=ON -DMGPS_BUILD_TOOLS=ON', archive: 'dashcam-gps-tools',      type: 'release']
]

def createCMakeBuild(Map os, Map task) {
    if (!os.builds.containsKey(task.type))
        return

    Map opts = [:]
    opts.installation = 'CMake-3.16'
    opts.buildDir = 'build'
    opts.cleanBuild = true

    Map type = os.builds[task.type]
    if (type.containsKey('build'))
        opts.buildType = type.build
    if (type.containsKey('generator'))
        opts.generator = type.generator
    if (type.containsKey('steps'))
        opts.steps = type.steps
    
    if (task.containsKey('args'))
        opts.cmakeArgs = task.args

    cmakeBuild( opts )
}

Map posix = [
    python: '',
    ext: 'tar.gz',
    call: { String script, String label -> sh script:script, label:label },
    builds: [
        release: [ build: 'Release', generator: 'Ninja',
            steps: [
                [ args: 'all' ], [ args: 'install', envVars: 'DESTDIR=${WORKSPACE}/artifacts' ]
            ]
        ]
    ]
]

Map windows = [
    python: 'py ',
    ext: 'zip',
    call: { String script, String label -> bat script:"@${script}", label:label },
    builds: [
        release: [
            steps: [
                [ args: '-- -nologo -m -v:m -p:Configuration=Release', withCmake: true ]
            ]
        ]
    ]
]

def platforms = [
    [name: 'Linux', node: 'linux', os: posix ],
    [name: 'Win64', node: 'windows', os: windows ],
    [name: 'MacOS', node: 'mac', os: posix ]
]

def createJob(Map platform, Map build) {
    Map os = platform.os
    String archiveExt = os.ext
    String nodeLabel = platform.node
    String stageName = "${build.name} (${platform.name})"
    return {
        stage("${stageName}") {
            node("${nodeLabel}") {
                echo "[${env.NODE_NAME}] ${WORKSPACE}"
                checkout scm
                createCMakeBuild(os, build)
                os.call("${os.python}tools/pack.py ${build.archive}", 'Pack archives')
                archiveArtifacts "artifacts/${build.archive}-*.${archiveExt}"
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

@Library('xmos_jenkins_shared_library@v0.32.0') _

getApproval()

pipeline {
  agent none
  environment {
    REPO = 'lib_xua'
    VIEW = getViewName(REPO)
    TOOLS_VERSION = "15.2.1"    // For unit tests
  }
  options {
    skipDefaultCheckout()
  }
  stages {
    stage('Basic tests') {
      agent {
        label 'x86_64 && linux'
      }
      stages {
        stage('Get view') {
          steps {
            xcorePrepareSandbox("${VIEW}", "${REPO}")
          }
        }
        stage('Library checks') {
          steps {
            xcoreLibraryChecks("${REPO}", false)
          }
        }
        stage('Testing') {
          failFast true
          parallel {
            stage('Tests') {
              steps {
                dir("${REPO}/tests"){
                  viewEnv(){
                    withVenv{
                      sh "xmake -C test_midi -j" // Xdist does not like building so do here
                      runPytest('--numprocesses=auto -vvv')
                    }
                  }
                }
              }
            }
            stage('Unity tests') {
              steps {
                dir("${REPO}/tests/xua_unit_tests") {
                  withTools("${env.TOOLS_VERSION}") {
                    withVenv {
                      withEnv(["XMOS_CMAKE_PATH=${WORKSPACE}/xcommon_cmake"]) {
                        sh "cmake -G 'Unix Makefiles' -B build"
                        sh 'xmake -C build -j'
                        runPython("pytest -s --junitxml=pytest_unity.xml")
                        junit "pytest_unity.xml"
                      }
                    }
                  }
                }
              }
            }
          }
        }
        stage('xCORE builds') {
          steps {
            dir("${REPO}") {
              xcoreAllAppNotesBuild('examples')
              dir("${REPO}") {
                runXdoc('doc')
              }
            }
            // Archive all the generated .pdf docs
            archiveArtifacts artifacts: "${REPO}/**/pdf/*.pdf", fingerprint: true, allowEmptyArchive: true
          }
        }
      }
      post {
        cleanup {
          xcoreCleanSandbox()
        }
      }
    }
    stage('Build host apps') {
      failFast true
      parallel {
        stage('Build Linux host app') {
          agent {
            label 'x86_64&&linux'
          }
          steps {
            xcorePrepareSandbox("${VIEW}", "${REPO}")
            dir("${REPO}/${REPO}/host/xmosdfu") {
              sh 'make -f Makefile.Linux64'
            }
          }
          post {
            cleanup {
              xcoreCleanSandbox()
            }
          }
        }
        stage('Build Mac host app') {
          agent {
            label 'x86_64&&macOS'
          }
          steps {
            xcorePrepareSandbox("${VIEW}", "${REPO}")
            dir("${REPO}/${REPO}/host/xmosdfu") {
              sh 'make -f Makefile.OSX64'
            }
            dir("${REPO}/host_usb_mixer_control") {
                sh 'make -f Makefile.OSX'
                sh 'mkdir OSX/x86'
                sh 'mv xmos_mixer OSX/x86/xmos_mixer'
                archiveArtifacts artifacts: "OSX/x86/xmos_mixer", fingerprint: true
            }
          }
          post {
            cleanup {
              xcoreCleanSandbox()
            }
          }
        }
        stage('Build Pi host app') {
          agent {
            label 'pi'
          }
          steps {
            dir("${REPO}") {
              checkout scm
              dir("${REPO}/host/xmosdfu") {
                sh 'make -f Makefile.Pi'
              }
            }
          }
          post {
            cleanup {
              xcoreCleanSandbox()
            }
          }
        }
        stage('Build Windows host app') {
          agent {
            label 'x86_64&&windows'
          }
          steps {
            dir("${REPO}") {
              checkout scm
              dir("${REPO}/host/xmosdfu") {
                withVS("vcvars32.bat") {
                  bat "nmake /f Makefile.Win32"
                }
              }
              dir("host_usb_mixer_control") {
                  withVS() {
                    bat 'msbuild host_usb_mixer_control.vcxproj /property:Configuration=Release /property:Platform=x64'
                  }
                  bat 'mkdir Win\\x64'
                  bat 'mv bin/Release/x64/host_usb_mixer_control.exe Win/x64/xmos_mixer.exe'
                  archiveArtifacts artifacts: "Win/x64/xmos_mixer.exe", fingerprint: true
              }
            }
          }
          post {
            cleanup {
              xcoreCleanSandbox()
            }
          }
        }
      }
    }
    stage('Update') {
      agent {
        label 'x86_64 && linux'
      }
      steps {
        updateViewfiles()
      }
      post {
        cleanup {
          xcoreCleanSandbox()
        }
      }
    }
  }
}

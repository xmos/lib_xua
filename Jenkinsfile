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
    stage('Build host apps') {
      parallel {
        stage('Build Linux host app') {
          agent {
            label 'x86_64&&linux'
          }
          steps {
            xcorePrepareSandbox("${VIEW}", "${REPO}")
            dir("${REPO}/${REPO}/host/xmosdfu") {
              sh 'cmake -B build'
              sh 'make -C build'
              sh 'mkdir -p Linux64'
              sh 'mv build/bin/xmosdfu Linux64/xmosdfu'
              archiveArtifacts artifacts: "Linux64/xmosdfu", fingerprint: true
            }
          }
          post {
            cleanup {
              xcoreCleanSandbox()
            }
          }
        }
        stage('Build Mac x86 host app') {
          agent {
            label 'x86_64&&macOS'
          }
          steps {
            xcorePrepareSandbox("${VIEW}", "${REPO}")
            dir("${REPO}/${REPO}/host/xmosdfu") {
              sh 'cmake -B build'
              sh 'make -C build'
              sh 'mkdir -p OSX/x86'
              sh 'mv build/bin/xmosdfu OSX/x86/xmosdfu'
              archiveArtifacts artifacts: "OSX/x86/xmosdfu", fingerprint: true
            }
            dir("${REPO}/host_usb_mixer_control") {
                sh 'make -f Makefile.OSX'
                sh 'mkdir -p OSX/x86'
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
        stage('Build Mac arm host app') {
          agent {
            label 'arm64&&macos'
          }
          steps {
            xcorePrepareSandbox("${VIEW}", "${REPO}")
            dir("${REPO}/${REPO}/host/xmosdfu") {
              sh 'cmake -B build'
              sh 'make -C build'
              sh 'mkdir -p OSX/arm64'
              sh 'mv build/bin/xmosdfu OSX/arm64/xmosdfu'
              archiveArtifacts artifacts: "OSX/arm64/xmosdfu", fingerprint: true
              dir("OSX/arm64") {
                stash includes: 'xmosdfu', name: 'macos_xmosdfu'
              }
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
                sh 'cmake -B build'
                sh 'make -C build'
                sh 'mkdir -p RPi'
                sh 'mv build/bin/xmosdfu RPi/xmosdfu'
                archiveArtifacts artifacts: "RPi/xmosdfu", fingerprint: true
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
                  bat "cmake -B build -G Ninja"
                  bat "ninja -C build"
                  bat 'mkdir win32 && cp build/bin/xmosdfu.exe win32/'
                  archiveArtifacts artifacts: "win32/xmosdfu.exe", fingerprint: true
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
    stage('Testing') {
      parallel {
        stage('Build and sim tests') {
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
            stage('xcore builds') {
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
            stage('Simulator tests') {
              steps {
                dir("${REPO}/tests") {
                  viewEnv() {
                    withVenv {
                      sh "xmake -C test_midi -j 8" // Xdist does not like building so do here

                      dir("xua_unit_tests") {
                        withEnv(["XMOS_CMAKE_PATH=${WORKSPACE}/xcommon_cmake"]) {
                          sh "cmake -G 'Unix Makefiles' -B build"
                          sh 'xmake -C build -j 8'
                        }
                      }

                      // Use "not dfu" keyword filter for now; restructure test directories when converted to XCommon CMake
                      sh "pytest -n auto -vvv -k \"not dfu\" --junitxml=pytest_sim.xml"
                    }
                  }
                }
              }
            }
          }
          post {
            always {
              junit "lib_xua/tests/pytest_sim.xml"
            }
            cleanup {
              xcoreCleanSandbox()
            }
          }
        }
        stage('MacOS HW tests') {
          agent {
            label 'macos && arm64 && usb_audio && xcore.ai-mcab'
          }
          stages {
            stage('Get View') {
              steps {
                xcorePrepareSandbox("${VIEW}", "${REPO}")
              }
            }
            stage('Hardware tests') {
              steps {
                dir("hardware_test_tools/xmosdfu") {
                  unstash "macos_xmosdfu"
                }
                dir("lib_xua/tests/xua_hw_tests") {
                  withTools("${env.TOOLS_VERSION}") {
                    withEnv(["XMOS_CMAKE_PATH=${WORKSPACE}/xcommon_cmake"]) {
                      sh "cmake -G 'Unix Makefiles' -B build"
                      sh "xmake -C build -j 8"
                    }
                    withVenv {
                      withXTAG(["usb_audio_mc_xcai_dut"]) { xtagIds ->
                        sh "pytest -v --junitxml=pytest_hw_mac.xml --xtag-id=${xtagIds[0]}"
                      }
                    }
                  }
                }
              }
            }
          }
          post {
            always {
              junit "lib_xua/tests/xua_hw_tests/pytest_hw_mac.xml"
            }
            cleanup {
              xcoreCleanSandbox()
            }
          }
        }
        stage('Windows HW tests') {
          agent {
            label 'windows11 && usb_audio && xcore.ai-mcab'
          }
          stages {
            stage('Get View') {
              steps {
                xcorePrepareSandbox("${VIEW}", "${REPO}")
              }
            }
            stage('Hardware tests') {
              steps {
                dir("lib_xua/tests/xua_hw_tests") {
                  withTools("${env.TOOLS_VERSION}") {
                    withEnv(["XMOS_CMAKE_PATH=${WORKSPACE}/xcommon_cmake"]) {
                      sh "cmake -G 'Unix Makefiles' -B build"
                      sh "xmake -C build"
                    }
                    withVenv {
                      withXTAG(["usb_audio_mc_xcai_dut"]) { xtagIds ->
                        sh "pytest -v --junitxml=pytest_hw_win.xml --xtag-id=${xtagIds[0]}"
                      }
                    }
                  }
                }
              }
            }
          }
          post {
            always {
              junit "lib_xua/tests/xua_hw_tests/pytest_hw_win.xml"
            }
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

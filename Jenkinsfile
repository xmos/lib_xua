// This file relates to internal XMOS infrastructure and should be ignored by external users

@Library('xmos_jenkins_shared_library@v0.43.1') _

getApproval()

pipeline {

  agent none

  parameters {
    string(
      name: 'TOOLS_VERSION',
      defaultValue: '15.3.1',
      description: 'The XTC tools version'
    )
    string(
      name: 'XMOSDOC_VERSION',
      defaultValue: 'v8.0.0',
      description: 'The xmosdoc version')

    string(
      name: 'INFR_APPS_VERSION',
      defaultValue: 'v3.1.1',
      description: 'The infr_apps version'
    )
    choice(
        name: 'TEST_LEVEL', choices: ['smoke', 'nightly'],
        description: 'The level of test coverage to run')
  }
  options {
    skipDefaultCheckout()
    timestamps()
    buildDiscarder(xmosDiscardBuildSettings(onlyArtifacts = false))
  }

  stages {
    stage('🏗️ Build and test') {
      agent {
        label 'documentation && x86_64 && linux'
      }
      stages {
        stage('Checkout') {
          steps {

            println "Stage running on ${env.NODE_NAME}"

            script {
              def (server, user, repo) = extractFromScmUrl()
              env.REPO_NAME = repo
            }
            dir(REPO_NAME){
              checkoutScmShallow()
            }
          }
        }  // stage('Checkout')

        stage('Examples build') {
          steps {
            dir("${REPO_NAME}/examples") {
              xcoreBuild()
            }
          }
        }
        stage('Repo checks') {
          steps {
            warnError("Repo checks failed")
            {
              runRepoChecks("${WORKSPACE}/${REPO_NAME}")
            }
          }
        }
        stage('Doc build') {
          steps {
            dir(REPO_NAME) {
              buildDocs()
            }
          }
        }
        stage("Archive Lib") {
          steps {
            archiveSandbox(REPO_NAME)
          }
        } //stage("Archive Lib")

        stage('Build HW tests') {
          steps {
            dir("${REPO_NAME}/tests/xua_hw_tests") {
              xcoreBuild()
              stash includes: '**/*.xe', name: 'hw_test_bin', useDefaultExcludes: false
            } // dir(REPO_NAME)
          } // steps
        } // stage('Build tests')
      } // stages
      post {
        cleanup {
          xcoreCleanSandbox()
        }
      }
    }  // stage 'Build and test'

    stage('Build host apps') {
      parallel {
        stage('Build Linux host app') {
          agent {
            label 'x86_64&&linux'
          }
          steps {
            println "Stage running on ${env.NODE_NAME}"

            dir(REPO_NAME) {
              checkoutScmShallow()
              dir("${REPO_NAME}/host/xmosdfu") {
                sh 'cmake -B build'
                sh 'make -C build'
                sh 'mkdir -p Linux64'
                sh 'mv build/xmosdfu Linux64/xmosdfu'
                archiveArtifacts artifacts: "Linux64/xmosdfu", fingerprint: true
              }
            }
          }
          post {
            cleanup {
              xcoreCleanSandbox()
            }
          }
        }  // Build Linux host app

        stage('Build Mac x86 host app') {
          agent {
            label 'x86_64&&macOS'
          }
          steps {
            println "Stage running on ${env.NODE_NAME}"

            dir(REPO_NAME) {
              checkoutScmShallow()
              dir("${REPO_NAME}/host/xmosdfu") {
                sh 'cmake -B build'
                sh 'make -C build'
                sh 'mkdir -p OSX/x86'
                sh 'mv build/xmosdfu OSX/x86/xmosdfu'
                archiveArtifacts artifacts: "OSX/x86/xmosdfu", fingerprint: true
              }
              dir("host_usb_mixer_control") {
                sh 'make -f Makefile.OSX'
                sh 'mkdir -p OSX/x86'
                sh 'mv xmos_mixer OSX/x86/xmos_mixer'
                archiveArtifacts artifacts: "OSX/x86/xmos_mixer", fingerprint: true
              }
            }
          }
          post {
            cleanup {
              xcoreCleanSandbox()
            }
          }
        }  // Build Mac x86 host app

        stage('Build Mac arm host app') {
          agent {
            label 'arm64&&macos'
          }
          steps {
            println "Stage running on ${env.NODE_NAME}"

            dir(REPO_NAME) {
              checkoutScmShallow()
              dir("${REPO_NAME}/host/xmosdfu") {
                sh 'cmake -B build'
                sh 'make -C build'
                sh 'mkdir -p OSX/arm64'
                sh 'mv build/xmosdfu OSX/arm64/xmosdfu'
                archiveArtifacts artifacts: "OSX/arm64/xmosdfu", fingerprint: true
               dir("OSX/arm64") {
                  stash includes: 'xmosdfu', name: 'macos_xmosdfu'
                }
              } // dir("${REPO_NAME}/host/xmosdfu")
              dir("tests/xua_hw_tests/test_control/host")
              {
                sh 'cmake -B build'
                sh 'make -C build'
                stash includes: 'build/host_control_test', name: 'host_control_test_bin_mac_arm', useDefaultExcludes: false
              } // dir("${REPO_NAME}/tests/xua_hw_tests/test_control/host")
            }
          }
          post {
            cleanup {
              xcoreCleanSandbox()
            }
          }
        }  // Build Mac arm host app

        stage('Build Pi host app') {
          agent {
            label 'pi'
          }
          steps {
            println "Stage running on ${env.NODE_NAME}"

            dir(REPO_NAME) {
              checkoutScmShallow()
              dir("${REPO_NAME}/host/xmosdfu") {
                sh 'cmake -B build'
                sh 'make -C build'
                sh 'mkdir -p RPi'
                sh 'mv build/xmosdfu RPi/xmosdfu'
                archiveArtifacts artifacts: "RPi/xmosdfu", fingerprint: true
              }
            }
          }
          post {
            cleanup {
              xcoreCleanSandbox()
            }
          }
        }  // Build Pi host app

        stage('Build Windows host app') {
          agent {
            label 'x86_64&&windows&&usb_audio'
          }
          steps {
            println "Stage running on ${env.NODE_NAME}"

            dir(REPO_NAME) {
              checkoutScmShallow()
              dir("${REPO_NAME}/host/xmosdfu") {
                withVS("vcvars32.bat") {
                  bat "cmake -B build -G Ninja"
                  bat "ninja -C build"
                  bat 'mkdir win32 && cp build/xmosdfu.exe win32/'
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
              dir("tests/xua_hw_tests/test_control/host")
              {
                withVS("vcvars32.bat") {
                  bat "cmake -B build -G Ninja"
                  bat "ninja -C build"
                  stash includes: 'build/host_control_test.exe', name: 'host_control_test_bin_windows', useDefaultExcludes: false
                }
              } // dir("${REPO_NAME}/tests/xua_hw_tests/test_control/host")
            }
          }
          post {
            cleanup {
              xcoreCleanSandbox()
            }
          }
        }  // Build Windows host app
      }
    }  // Build host apps

    stage('Testing') {
      parallel {
        stage('Simulator and unit tests') {
          agent {
            label 'x86_64 && linux'
          }
          steps {
            dir(REPO_NAME) {
              checkoutScmShallow()

              withTools(params.TOOLS_VERSION) {
                dir("tests") {
                  createVenv(reqFile: "requirements.txt")
                  withVenv {
                    dir("xua_sim_tests") {
                      sh 'cmake -G "Unix Makefiles" -B build'

                      // XEs for MIDI tests are shared, so need to be built first
                      sh 'xmake -C build test_midi_loopback test_midi_xs2 test_midi_xs3'

                      sh "pytest -v -n auto --junitxml=pytest_sim.xml"
                    }

                    dir("xua_unit_tests") {
                      xcoreBuild()
                      runPytest()
                    }
                  }
                }
              }
            }
          }
          post {
            always {
              junit "${REPO_NAME}/tests/**/pytest_*.xml"
            }
            cleanup {
              xcoreCleanSandbox()
            }
          }
        }  // Simulator and unit tests

        stage('MacOS HW tests') {
          agent {
            label 'macos && arm64 && usb_audio && xcore.ai-mcab'
          }
          steps {
            println "Stage running on ${env.NODE_NAME}"

            dir(REPO_NAME) {
              checkoutScmShallow()
            }

            dir("${REPO_NAME}/tests/xua_hw_tests") {
              unstash "hw_test_bin" // Unstash HW test DUT binaries
            }
            dir("${REPO_NAME}/tests/xua_hw_tests/test_control/host")
            {
              unstash "host_control_test_bin_mac_arm" // Unstash host app for control test
            }
            dir("${REPO_NAME}/tests") {
              createVenv(reqFile: "requirements.txt")
              withTools(params.TOOLS_VERSION) {
                withVenv {
                  // Change directory into xmosdfu and unstash
                  // Note hardcoded path to hardware_test_tools since it's installed as an editable requirement in requirements.txt
                  dir("${env.VIRTUAL_ENV}/src/hardware-test-tools/xmosdfu") {
                    unstash "macos_xmosdfu"
                  }
                  dir("xua_hw_tests") {
                    withXTAG(["usb_audio_mc_xcai_dut"]) { xtagIds ->
                      sh "pytest -v --junitxml=pytest_hw_mac.xml --xtag-id=${xtagIds[0]} --level ${params.TEST_LEVEL}"
                    }
                  } // dir("xua_hw_tests")
                } // withVenv
              } // withTools(params.TOOLS_VERSION)
            } // dir("${REPO_NAME}/tests")
          } // steps
          post {
            always {
              junit "${REPO_NAME}/tests/xua_hw_tests/pytest_hw_mac.xml"
            }
            cleanup {
              xcoreCleanSandbox()
            }
          }
        }  // MacOS HW tests

        stage('Windows HW tests') {
          agent {
            label 'windows11 && usb_audio && xcore.ai-mcab'
          }
          steps {
            println "Stage running on ${env.NODE_NAME}"

            dir(REPO_NAME) {
              checkoutScmShallow()
            }

            dir("${REPO_NAME}/tests/xua_hw_tests") {
              unstash "hw_test_bin" // Unstash HW test DUT binaries
            }
            dir("${REPO_NAME}/tests/xua_hw_tests/test_control/host")
            {
              unstash "host_control_test_bin_windows" // Unstash host app for control test
            }

            dir("${REPO_NAME}/tests") {
              createVenv(reqFile: "requirements.txt")
              withTools(params.TOOLS_VERSION) {
                dir("xua_hw_tests") {
                  withVenv {
                    withXTAG(["usb_audio_mc_xcai_dut"]) { xtagIds ->
                      sh "pytest -v --junitxml=pytest_hw_win.xml --xtag-id=${xtagIds[0]} --level ${params.TEST_LEVEL}"
                    }
                  }
                }
              }
            }
          }
          post {
            always {
              junit "${REPO_NAME}/tests/xua_hw_tests/pytest_hw_win.xml"
            }
            cleanup {
              xcoreCleanSandbox()
            }
          }
        }  // Windows HW tests
      }
    }  // stage('Testing')
    stage('🚀 Release') {
      when {
      expression { triggerRelease.isReleasable() }
      }
      steps {
        triggerRelease()
      }
    }
  } // stages
} // pipeline

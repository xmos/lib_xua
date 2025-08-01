// This file relates to internal XMOS infrastructure and should be ignored by external users

@Library('xmos_jenkins_shared_library@v0.39.0') _

getApproval()

pipeline {
  agent none
  environment {
    REPO = 'lib_xua'
    REPO_NAME = 'lib_xua'
  }
  options {
    buildDiscarder(xmosDiscardBuildSettings())
    skipDefaultCheckout()
    timestamps()
  }
  parameters {
    string(
      name: 'TOOLS_VERSION',
      defaultValue: '15.3.1',
      description: 'The XTC tools version'
    )
    string(
      name: 'XMOSDOC_VERSION',
      defaultValue: 'v7.3.0',
      description: 'The xmosdoc version')

    string(
      name: 'INFR_APPS_VERSION',
      defaultValue: 'v2.1.0',
      description: 'The infr_apps version'
    )
    choice(
        name: 'TEST_LEVEL', choices: ['smoke', 'nightly'],
        description: 'The level of test coverage to run')
  }

  stages {
    stage('Build and test') {
      agent {
        label 'documentation && x86_64 && linux'
      }
      stages {
        stage('Checkout and lib checks') {
          steps {
            println "Stage running on ${env.NODE_NAME}"
            dir("${REPO}") {
              checkoutScmShallow()
              dir("examples") {
                withTools(params.TOOLS_VERSION) {
                  xcoreBuild()
                }
              }
            }
            runLibraryChecks("${WORKSPACE}/${REPO}", "${params.INFR_APPS_VERSION}")
          }
        }  // stage('Checkout and lib checks')
        stage("Archive Lib") {
          steps {
            archiveSandbox(REPO)
          }
        } //stage("Archive Lib")

        stage('Build examples') {
          steps {
            println "Stage running on ${env.NODE_NAME}"

            dir("${REPO}") {
              dir("examples") {
                withTools(params.TOOLS_VERSION) {
                  xcoreBuild()
                }
              }
            }
            // Archive all the generated .xe files
            archiveArtifacts artifacts: "${REPO}/examples/**/*.xe"
          }
        }  // Build examples

        stage('Build HW tests') {
          steps {
            dir("${REPO}") {
                withTools(params.TOOLS_VERSION) {
                  dir("tests/xua_hw_tests") {
                    xcoreBuild()
                    stash includes: '**/*.xe', name: 'hw_test_bin', useDefaultExcludes: false
                  }
                } // withTools(params.TOOLS_VERSION)
            } // dir("${REPO}")
          } // steps
        } // stage('Build tests')

        stage('Build Documentation') {
          steps {
            dir("${REPO}") {
                buildDocs()
                dir("examples/AN00246_xua_example") {
                  buildDocs()
                }
                dir("examples/AN00247_xua_example_spdif_tx") {
                  buildDocs()
                }
                dir("examples/AN00248_xua_example_pdm_mics") {
                  buildDocs()
              }
            } // dir("${REPO}")
          } // steps
        } // stage('Build Documentation')
      }
      post {
        cleanup {
          xcoreCleanSandbox()
        }
      }
    }  // Build and test

    stage('Build host apps') {
      parallel {
        stage('Build Linux host app') {
          agent {
            label 'x86_64&&linux'
          }
          steps {
            println "Stage running on ${env.NODE_NAME}"

            dir("${REPO}") {
              checkoutScmShallow()
              dir("${REPO}/host/xmosdfu") {
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

            dir("${REPO}") {
              checkoutScmShallow()
              dir("${REPO}/host/xmosdfu") {
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

            dir("${REPO}") {
              checkoutScmShallow()
              dir("${REPO}/host/xmosdfu") {
                sh 'cmake -B build'
                sh 'make -C build'
                sh 'mkdir -p OSX/arm64'
                sh 'mv build/xmosdfu OSX/arm64/xmosdfu'
                archiveArtifacts artifacts: "OSX/arm64/xmosdfu", fingerprint: true
               dir("OSX/arm64") {
                  stash includes: 'xmosdfu', name: 'macos_xmosdfu'
                }
              } // dir("${REPO}/host/xmosdfu")
              dir("tests/xua_hw_tests/test_control/host")
              {
                sh 'pwd'
                sh 'ls -lrt '
                sh 'cmake -B build'
                sh 'make -C build'
                stash includes: 'build/host_control_test', name: 'host_control_test_bin_mac_arm', useDefaultExcludes: false
              } // dir("${REPO}/tests/xua_hw_tests/test_control/host")
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

            dir("${REPO}") {
              checkoutScmShallow()
              dir("${REPO}/host/xmosdfu") {
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

            dir("${REPO}") {
              checkoutScmShallow()
              dir("${REPO}/host/xmosdfu") {
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
              } // dir("${REPO}/tests/xua_hw_tests/test_control/host")
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
            dir("${REPO}") {
              checkoutScmShallow()

              withTools(params.TOOLS_VERSION) {
                dir("tests") {
                  createVenv(reqFile: "requirements.txt")
                  withVenv {
                    // Cross-product of all parameters in test_i2s_loopback produces invalid configs
                    // which cannot be built. They are skipped in pytest, but the build failures
                    // prevent all the XEs being built before running pytest.
                    dir("xua_sim_tests") {
                      sh 'cmake -G "Unix Makefiles" -B build'

                      // XEs for MIDI tests are shared, so need to be built first
                      sh 'xmake -C build test_midi_loopback test_midi_xs2 test_midi_xs3'

                      sh "pytest -v -n auto --junitxml=pytest_sim.xml"
                    }

                    dir("xua_unit_tests") {
                      sh "cmake -G 'Unix Makefiles' -B build"
                      sh 'xmake -C build -j 16'
                      sh "pytest -v -n auto --junitxml=pytest_unit.xml"
                    }
                  }
                }
              }
            }
          }
          post {
            always {
              junit "${REPO}/tests/**/pytest_*.xml"
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

            dir("${REPO}") {
              checkoutScmShallow()
            }

            dir("hardware_test_tools/xmosdfu") {
              unstash "macos_xmosdfu"
            }
            dir("${REPO}/tests/xua_hw_tests") {
              unstash "hw_test_bin" // Unstash HW test DUT binaries
            }
            dir("${REPO}/tests/xua_hw_tests/test_control/host")
            {
              unstash "host_control_test_bin_mac_arm" // Unstash host app for control test
            }
            dir("${REPO}/tests") {
              createVenv(reqFile: "requirements.txt")
              withTools(params.TOOLS_VERSION) {
                dir("xua_hw_tests") {
                  withVenv {
                    withXTAG(["usb_audio_mc_xcai_dut"]) { xtagIds ->
                      sh "pytest -v --junitxml=pytest_hw_mac.xml --xtag-id=${xtagIds[0]} --level ${params.TEST_LEVEL}"
                    }
                  }
                }
              }
            }
          }
          post {
            always {
              junit "${REPO}/tests/xua_hw_tests/pytest_hw_mac.xml"
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

            dir("${REPO}") {
              checkoutScmShallow()
            }

            dir("${REPO}/tests/xua_hw_tests") {
              unstash "hw_test_bin" // Unstash HW test DUT binaries
            }
            dir("${REPO}/tests/xua_hw_tests/test_control/host")
            {
              unstash "host_control_test_bin_windows" // Unstash host app for control test
            }

            dir("${REPO}/tests") {
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
              junit "${REPO}/tests/xua_hw_tests/pytest_hw_win.xml"
            }
            cleanup {
              xcoreCleanSandbox()
            }
          }
        }  // Windows HW tests
      }
    }  // Testing
  }
}

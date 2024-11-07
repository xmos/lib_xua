// This file relates to internal XMOS infrastructure and should be ignored by external users

@Library('xmos_jenkins_shared_library@v0.34.0') _

def clone_test_deps() {
  dir("${WORKSPACE}") {
    sh "git clone git@github.com:xmos/test_support"
    sh "git -C test_support checkout 961532d89a98b9df9ccbce5abd0d07d176ceda40"

    sh "git clone git@github0.xmos.com:xmos-int/xtagctl"
    sh "git -C xtagctl checkout v2.0.0"

    sh "git clone git@github.com:xmos/hardware_test_tools"
    sh "git -C hardware_test_tools checkout 2f9919c956f0083cdcecb765b47129d846948ed4"
  }
}

getApproval()

pipeline {
  agent none
  environment {
    REPO = 'lib_xua'
  }
  options {
    buildDiscarder(xmosDiscardBuildSettings())
    skipDefaultCheckout()
    timestamps()
  }
  parameters {
    string(
      name: 'TOOLS_VERSION',
      defaultValue: '15.3.0',
      description: 'The XTC tools version'
    )
    string(
      name: 'XMOSDOC_VERSION',
      defaultValue: 'v6.1.3',
      description: 'The xmosdoc version')
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
              checkout scm
              dir("examples") {
                withTools(params.TOOLS_VERSION) {
                  sh 'cmake -G "Unix Makefiles" -B build'
                }
              }
            }
            warnError("Docs") {
              runLibraryChecks("${WORKSPACE}/${REPO}", "v2.0.1")
            }
          }
        }  // Checkout and lib checks

        stage('Build examples') {
          steps {
            println "Stage running on ${env.NODE_NAME}"

            dir("${REPO}") {
              dir("examples") {
                withTools(params.TOOLS_VERSION) {
                  sh 'cmake -G "Unix Makefiles" -B build'
                  sh 'xmake -C build -j 16'
                }
              }
            }
            // Archive all the generated .xe files
            archiveArtifacts artifacts: "${REPO}/examples/**/*.xe"
          }
        }  // Build examples

        stage('Build Documentation') {
          steps {
            dir("${REPO}") {
              warnError("Docs") {
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
              checkout scm
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
              checkout scm
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
              checkout scm
              dir("${REPO}/host/xmosdfu") {
                sh 'cmake -B build'
                sh 'make -C build'
                sh 'mkdir -p OSX/arm64'
                sh 'mv build/xmosdfu OSX/arm64/xmosdfu'
                archiveArtifacts artifacts: "OSX/arm64/xmosdfu", fingerprint: true
               dir("OSX/arm64") {
                  stash includes: 'xmosdfu', name: 'macos_xmosdfu'
                }
              }
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
              checkout scm
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
              checkout scm
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
              checkout scm

              clone_test_deps()

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

            clone_test_deps()

            dir("${REPO}") {
              checkout scm
            }

            dir("hardware_test_tools/xmosdfu") {
              unstash "macos_xmosdfu"
            }

            dir("${REPO}/tests") {
              createVenv(reqFile: "requirements.txt")
              withTools(params.TOOLS_VERSION) {
                dir("xua_hw_tests") {
                  sh "cmake -G 'Unix Makefiles' -B build"
                  sh "xmake -C build -j 8"

                  withVenv {
                    withXTAG(["usb_audio_mc_xcai_dut"]) { xtagIds ->
                      sh "pytest -v --junitxml=pytest_hw_mac.xml --xtag-id=${xtagIds[0]}"
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

            clone_test_deps()

            dir("${REPO}") {
              checkout scm
            }

            dir("${REPO}/tests") {
              createVenv(reqFile: "requirements.txt")
              withTools(params.TOOLS_VERSION) {
                dir("xua_hw_tests") {
                  sh "cmake -G 'Unix Makefiles' -B build"
                  sh "xmake -C build"

                  withVenv {
                    withXTAG(["usb_audio_mc_xcai_dut"]) { xtagIds ->
                      sh "pytest -v --junitxml=pytest_hw_win.xml --xtag-id=${xtagIds[0]}"
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

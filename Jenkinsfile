// This file relates to internal XMOS infrastructure and should be ignored by external users

@Library('xmos_jenkins_shared_library@feature/build_doc_warnings') _

def clone_test_deps() {
  dir("${WORKSPACE}") {
    sh "git clone git@github.com:xmos/test_support"
    sh "git -C test_support checkout 961532d89a98b9df9ccbce5abd0d07d176ceda40"

    sh "git clone git@github0.xmos.com:xmos-int/xtagctl"
    sh "git -C xtagctl checkout v2.0.0"

    sh "git clone git@github.com:xmos/hardware_test_tools"
    sh "git -C hardware_test_tools checkout develop"
  }
}

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
  }
}

@Library('xmos_jenkins_shared_library@v0.21.0') _

getApproval()

pipeline {
  agent none
  environment {
    REPO = 'lib_xua'
    VIEW = getViewName(REPO)
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
                      runPytest('--numprocesses=4')
                    }
                  }
                }
              }
            }
            stage('Unity tests') {
              steps {
                dir("${REPO}") {
                  dir('tests') {
                    dir('xua_unit_tests') {
                      withVenv {
                        runWaf('.', "configure clean build --target=xcore200")
                        viewEnv() {
                          runPython("TARGET=XCORE200 pytest -s")
                        }
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
                runVS('nmake /f Makefile.Win32')
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

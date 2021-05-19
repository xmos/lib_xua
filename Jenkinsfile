@Library('xmos_jenkins_shared_library@v0.16.2') _

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
        label 'x86_64&&brew'
      }
      stages {
        stage('Get view') {
          steps {
            xcorePrepareSandbox("${VIEW}", "${REPO}")
          }
        }
        stage('Library checks') {
          steps {
            xcoreLibraryChecks("${REPO}")
          }
        }
        stage('Legacy tests') {
          steps {
            runXmostest("${REPO}", 'legacy_tests')
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
            label 'x86_64&&brew&&linux'
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
            label 'x86_64&&brew&&macOS'
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
        label 'x86_64&&brew'
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

@Library('xmos_jenkins_shared_library@develop') _

getApproval()

pipeline {
  agent none
  environment {
    REPO = 'lib_xua'
    VIEW = "${env.JOB_NAME.contains('PR-') ? REPO+'_'+env.CHANGE_TARGET : REPO+'_'+env.BRANCH_NAME}"
  }
  triggers {
    /* Trigger this Pipeline on changes to the repos dependencies
     *
     * If this Pipeline is running in a pull request, the triggers are set
     * on the base branch the PR is set to merge in to.
     *
     * Otherwise the triggers are set on the branch of a matching name to the
     * one this Pipeline is on.
     */
    upstream(
      upstreamProjects:
        (env.JOB_NAME.contains('PR-') ?
          "../lib_device_control/${env.CHANGE_TARGET}," +
          "../lib_dsp/${env.CHANGE_TARGET}," +
          "../lib_i2c/${env.CHANGE_TARGET}," +
          "../lib_logging/${env.CHANGE_TARGET}," +
          "../lib_mic_array/${env.CHANGE_TARGET}," +
          "../lib_spdif/${env.CHANGE_TARGET}," +
          "../lib_xassert/${env.CHANGE_TARGET}," +
          "../lib_xud/${env.CHANGE_TARGET}," +
          "../tools_released/${env.CHANGE_TARGET}," +
          "../tools_xmostest/${env.CHANGE_TARGET}," +
          "../xdoc_released/${env.CHANGE_TARGET}"
        :
          "../lib_device_control/${env.BRANCH_NAME}," +
          "../lib_dsp/${env.BRANCH_NAME}," +
          "../lib_i2c/${env.BRANCH_NAME}," +
          "../lib_logging/${env.BRANCH_NAME}," +
          "../lib_mic_array/${env.BRANCH_NAME}," +
          "../lib_spdif/${env.BRANCH_NAME}," +
          "../lib_xassert/${env.BRANCH_NAME}," +
          "../lib_xud/${env.BRANCH_NAME}," +
          "../tools_released/${env.BRANCH_NAME}," +
          "../tools_xmostest/${env.BRANCH_NAME}," +
          "../xdoc_released/${env.BRANCH_NAME}"),
      threshold: hudson.model.Result.SUCCESS
    )
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
        stage('Tests') {
          steps {
            runXmostest("${REPO}", 'tests')
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

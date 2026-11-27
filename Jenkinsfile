// 내부 정기 점검 파이프라인(Jenkins): 빌드 성공 + 소스코드 품질 기준(CLAUDE.md 2장)을 모두 만족해야 통과한다.
// 모든 단계는 개발 환경과 같은 claudeclangbuild 이미지 안에서 실행한다.
// 한 단계가 실패해도 나머지 단계를 실행해 모든 결과를 보고하고, 최종 결과는 실패로 처리한다.
pipeline {
    agent {
        docker {
            image 'claudeclangbuild'
            reuseNode true
        }
    }

    // 변경이 없어도 주기적으로(약 4시간마다) 실행해 내부 환경에서 품질 기준을 재검증한다.
    // GitHub Actions(.github/workflows/ci.yml)가 PR 머지 조건이고, Jenkins는 내부 정기 점검용이다.
    triggers {
        cron('H H/4 * * *')
    }

    options {
        timestamps()
        timeout(time: 30, unit: 'MINUTES')
        buildDiscarder(logRotator(numToKeepStr: '20'))
    }

    stages {
        stage('Build') {
            steps {
                sh 'bash scripts/ci/build.sh'
            }
        }

        stage('Test') {
            steps {
                catchError(buildResult: 'FAILURE', stageResult: 'FAILURE') {
                    sh 'bash scripts/ci/test.sh'
                }
            }
            post {
                always {
                    // tdd / unit / integration / system 결과가 파일별로 분리되어 있다
                    junit allowEmptyResults: true, testResults: 'reports/test/*.xml'
                }
            }
        }

        stage('Coverage') {
            steps {
                catchError(buildResult: 'FAILURE', stageResult: 'FAILURE') {
                    sh 'bash scripts/ci/coverage.sh'
                }
            }
            post {
                always {
                    // 스위트별로 따로 기록한다 (TDD 커버리지와 단위 테스트 커버리지는 합산하지 않는다)
                    // 결과 파일이 있는 스위트만 기록한다 (아직 없는 스위트는 건너뛴다)
                    script {
                        [[id: 'unit', name: 'Unit Test Coverage'],
                         [id: 'tdd', name: 'TDD Test Coverage'],
                         [id: 'integration', name: 'Integration Test Coverage']].each { suite ->
                            def report = "reports/coverage/${suite.id}/cobertura.xml"
                            if (fileExists(report)) {
                                recordCoverage(
                                    id: "${suite.id}-coverage", name: suite.name,
                                    tools: [[parser: 'COBERTURA', pattern: report]],
                                    failOnError: false, skipPublishingChecks: true)
                            }
                        }
                    }
                    // MC/DC 상세는 LLVM HTML 리포트로 확인한다
                    publishHTML(target: [
                        reportName: 'Unit MC/DC Coverage (LLVM)', reportDir: 'reports/coverage/unit/html',
                        reportFiles: 'index.html', keepAll: true, allowMissing: true, alwaysLinkToLastBuild: true])
                    publishHTML(target: [
                        reportName: 'TDD Coverage (LLVM)', reportDir: 'reports/coverage/tdd/html',
                        reportFiles: 'index.html', keepAll: true, allowMissing: true, alwaysLinkToLastBuild: true])
                }
            }
        }

        stage('Static Analysis') {
            steps {
                catchError(buildResult: 'FAILURE', stageResult: 'FAILURE') {
                    sh 'bash scripts/ci/static.sh'
                }
            }
            post {
                always {
                    recordIssues(
                        tool: cppCheck(pattern: 'reports/cppcheck.xml'),
                        qualityGates: [[threshold: 1, type: 'TOTAL', criticality: 'FAILURE']],
                        skipPublishingChecks: true)
                }
            }
        }

        stage('Metrics') {
            steps {
                catchError(buildResult: 'FAILURE', stageResult: 'FAILURE') {
                    sh 'bash scripts/ci/metrics.sh'
                }
            }
            post {
                always {
                    publishHTML(target: [
                        reportName: 'Lizard (CCN / Function Length)', reportDir: 'reports',
                        reportFiles: 'lizard.html', keepAll: true, allowMissing: true, alwaysLinkToLastBuild: true])
                }
            }
        }

        stage('Comments') {
            steps {
                catchError(buildResult: 'FAILURE', stageResult: 'FAILURE') {
                    sh 'python3 scripts/ci/check_comments.py'
                    sh 'bash scripts/ci/docs.sh'
                }
            }
            post {
                always {
                    publishHTML(target: [
                        reportName: 'Doxygen', reportDir: 'reports/doxygen/html',
                        reportFiles: 'index.html', keepAll: true, allowMissing: true, alwaysLinkToLastBuild: true])
                }
            }
        }
    }

    post {
        always {
            archiveArtifacts artifacts: 'reports/**', allowEmptyArchive: true
        }
    }
}

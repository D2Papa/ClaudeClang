# claudeclangbuild: 개발 빌드와 CI 빌드가 함께 쓰는 단일 빌드 이미지
#   docker build -t claudeclangbuild .
#   docker build -t claudeclangbuild --build-arg LLVM_VERSION=21 .   # Clang 버전 변경
FROM ubuntu:24.04

ARG LLVM_VERSION=22
ARG PICT_VERSION=v3.7.4

ENV DEBIAN_FRONTEND=noninteractive \
    LANG=C.UTF-8 \
    LLVM_VERSION=${LLVM_VERSION} \
    PATH=/opt/venv/bin:$PATH

# 기본 도구: CMake, Ninja, Cppcheck, GoogleTest/GoogleMock, CLOC, Doxygen, Python
RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates curl wget gnupg git make g++ \
        cmake ninja-build cppcheck cloc doxygen graphviz \
        libgtest-dev libgmock-dev \
        python3 python3-venv \
    && rm -rf /var/lib/apt/lists/*

# LLVM/Clang (apt.llvm.org): 컴파일러, 커버리지(llvm-profdata/llvm-cov, MC/DC), clang-format, clang-tidy
RUN wget -qO- https://apt.llvm.org/llvm-snapshot.gpg.key | gpg --dearmor -o /usr/share/keyrings/llvm.gpg \
    && echo "deb [signed-by=/usr/share/keyrings/llvm.gpg] http://apt.llvm.org/noble/ llvm-toolchain-noble-${LLVM_VERSION} main" \
        > /etc/apt/sources.list.d/llvm.list \
    && apt-get update && apt-get install -y --no-install-recommends \
        clang-${LLVM_VERSION} llvm-${LLVM_VERSION} \
        clang-format-${LLVM_VERSION} clang-tidy-${LLVM_VERSION} \
        libclang-rt-${LLVM_VERSION}-dev \
    && rm -rf /var/lib/apt/lists/* \
    && for tool in clang clang++ clang-format clang-tidy llvm-cov llvm-profdata; do \
         ln -sf /usr/bin/${tool}-${LLVM_VERSION} /usr/local/bin/${tool}; \
       done

# Python 도구: Lizard(순환복잡도/함수 라인 수, HTML 출력에 jinja2 필요), lcov_cobertura(커버리지 XML 변환)
RUN python3 -m venv /opt/venv \
    && pip install --no-cache-dir lizard jinja2 lcov_cobertura

# PICT (페어와이즈 테스트 조합 생성)
RUN git clone --depth 1 --branch ${PICT_VERSION} https://github.com/microsoft/pict.git /tmp/pict \
    && make -C /tmp/pict -j"$(nproc)" \
    && install -m 0755 /tmp/pict/pict /usr/local/bin/pict \
    && rm -rf /tmp/pict

WORKDIR /work
CMD ["bash"]

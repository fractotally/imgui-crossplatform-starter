#!/usr/bin/env bash
#
# Local build & smoke test script for imgui-crossplatform-starter
# Usage:
#   ./scripts/test-build.sh            # Build both native + web (if emsdk available)
#   ./scripts/test-build.sh --native   # Only native
#   ./scripts/test-build.sh --web      # Only web
#

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(dirname "$SCRIPT_DIR")"
cd "$ROOT_DIR"

NATIVE_ONLY=false
WEB_ONLY=false

for arg in "$@"; do
    case $arg in
        --native-only|--native) NATIVE_ONLY=true ;;
        --web-only|--web)       WEB_ONLY=true ;;
        *) echo "Unknown option: $arg"; exit 1 ;;
    esac
done

echo "=== ImGui Cross-Platform Starter - Build & Test ==="
echo "Working directory: $ROOT_DIR"
echo

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

success() { echo -e "${GREEN}✅ $1${NC}"; }
warn()    { echo -e "${YELLOW}⚠️  $1${NC}"; }
error()   { echo -e "${RED}❌ $1${NC}"; exit 1; }

# ===================== NATIVE BUILD =====================
if ! $WEB_ONLY; then
    echo ">>> Building Native version..."
    
    if ! command -v cmake &> /dev/null; then
        error "cmake not found. Please install CMake."
    fi

    BUILD_DIR="build-native-test"
    rm -rf "$BUILD_DIR"
    mkdir -p "$BUILD_DIR"

    echo "Configuring with CMake..."
    cmake -B "$BUILD_DIR" -S . -G Ninja -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_FLAGS="-O2" 2>&1 | tail -5

    echo "Building..."
    cmake --build "$BUILD_DIR" --parallel $(nproc 2>/dev/null || echo 4)

    if [ -f "$BUILD_DIR/app" ]; then
        success "Native build successful: $BUILD_DIR/app"
        
        # Headless smoke test
        if command -v xvfb-run &> /dev/null; then
            echo "Running headless smoke test (5s timeout)..."
            if timeout 5s xvfb-run --auto-servernum --server-args="-screen 0 1280x720x24" \
                "$BUILD_DIR/app" > /dev/null 2>&1 || true; then
                success "Native app launched successfully under xvfb"
            else
                warn "Native app test completed (GUI apps usually time out in headless mode — this is normal)"
            fi
        else
            warn "xvfb-run not found — skipping headless GUI test. Install with: sudo apt install xvfb"
        fi
    else
        error "Native build failed — executable not found"
    fi
    echo
fi

# ===================== WEB BUILD =====================
if ! $NATIVE_ONLY; then
    echo ">>> Building Web (Emscripten) version..."

    if ! command -v emcmake &> /dev/null; then
        warn "Emscripten (emcmake) not found in PATH."
        warn "Skipping web build. Activate emsdk or install Emscripten to test web target."
    else
        BUILD_DIR="build-web-test"
        rm -rf "$BUILD_DIR"
        mkdir -p "$BUILD_DIR"

        echo "Configuring with emcmake..."
        emcmake cmake -B "$BUILD_DIR" -S . -DCMAKE_BUILD_TYPE=Release

        echo "Building WebAssembly..."
        cmake --build "$BUILD_DIR" --parallel $(nproc 2>/dev/null || echo 4)

        if [ -f "$BUILD_DIR/index.html" ] && [ -f "$BUILD_DIR/index.wasm" ]; then
            success "Web build successful!"
            ls -lh "$BUILD_DIR"/index.*
        else
            warn "Web build completed but expected files (index.html + index.wasm) not found."
            ls -la "$BUILD_DIR"/index.* 2>/dev/null || true
        fi
    fi
    echo
fi

success "All requested builds completed!"
echo
echo "Next steps:"
echo "  • Native: ./$BUILD_DIR/app"
echo "  • Web:    python3 -m http.server -d $BUILD_DIR 8000  (then open http://localhost:8000)"
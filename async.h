#pragma once

namespace async {
    using context_t = void*;

    context_t connect(size_t block_size);
    void receive(context_t, const char* data, size_t size);
    void disconnect(context_t);
};
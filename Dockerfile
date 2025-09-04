# -------- BUILD --------
FROM ubuntu:22.04 AS build

# Установка зависимостей для сборки
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libssl-dev \
    libhiredis-dev \
    libcurl4-openssl-dev \
    zlib1g-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

# Сборка проекта
RUN mkdir -p build && cd build \
    && cmake -S .. -B . -DCMAKE_BUILD_TYPE=Release \
    && cmake --build . -j$(nproc)

# Сбор всех .so файлов из /app/build/ и подкаталогов в /app/build/libs/
RUN mkdir -p /app/build/libs && \
    find /app/build -type f -name "*.so*" -exec cp {} /app/build/libs/ \; 2>/dev/null || true

# -------- RUNTIME --------
FROM ubuntu:22.04

# Установка runtime-зависимостей
RUN apt-get update && apt-get install -y \
    libhiredis0.14 \
    redis-server \
    libcurl4 \
    zlib1g \
    libssl3 \
    redis-server \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Копирование исполняемого файла
COPY --from=build /app/build/bin/caching-proxy /usr/local/bin/

# Копирование всех .so файлов из /app/build/libs/
COPY --from=build /app/build/libs/*.so* /usr/local/lib/
COPY --from=build /app/redis_files/ /etc/caching-proxy/

# Обновление кэша библиотек
RUN ldconfig

# Открытие порта
EXPOSE 3600

# Запуск приложения
ENTRYPOINT ["/usr/local/bin/caching-proxy"]
CMD ["--port", "3600", "--origin", "https://cppreference.com"]

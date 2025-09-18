FROM devkitpro/devkitarm:latest
WORKDIR /balatro

CMD ["sh", "-c", "make -j$(nproc)"]
# Use an official Ubuntu as a base image
FROM ubuntu:latest

# Set the working directory in the container
WORKDIR /rma

# Copy the current directory contents into the container
COPY . /rma

# Install build essentials for C++ compilation
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    gdb  # if you need debugging tools

# The container does not need to keep running, so use an idle command.
CMD ["tail", "-f", "/dev/null"]

#include <iostream>

#include <CLI/CLI.hpp>

int main(int argc, char *argv[])
{
  {
    CLI::App app{"ljit: leech JIT"};

    CLI11_PARSE(app, argc, argv);
  }
}

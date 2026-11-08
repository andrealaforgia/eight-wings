#!/usr/bin/env python3
"""Run every generator in tools/assetgen (gen_*.py, each exposing generate()) deterministically.

Usage: python3 tools/gen_assets.py [module ...]     (default: all)
"""
import importlib
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "assetgen"))


def main():
    names = sys.argv[1:] or sorted(f[:-3] for f in os.listdir(os.path.join(HERE, "assetgen"))
                                   if f.startswith("gen_") and f.endswith(".py"))
    for n in names:
        mod = importlib.import_module(n)
        print("generating", n)
        mod.generate()


if __name__ == "__main__":
    main()

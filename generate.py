import random
import argparse


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-c', '--count', type=int, required=True)
    parser.add_argument('-o', '--output', type=str, required=True)
    args = parser.parse_args()

    with open(args.output, 'wb') as f:
        for _ in range(args.count):
            # in range 1 - 1000
            f.write(random.randint(1, 1000).to_bytes(4, 'little'))


if __name__ == "__main__":
    main()
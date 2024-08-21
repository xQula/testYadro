import argparse


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('filename', type=str)
    parser.add_argument('--verify', action='store_true', default=True)
    parser.add_argument('--no-verify', dest='verify', action='store_false')
    args = parser.parse_args()

    with open(args.filename, 'rb') as f:
        lst = []
        while True:
            chunk = f.read(4)
            if not chunk:
                break
            lst.append(int.from_bytes(chunk, 'little'))
    print(f"Tape: [ {''.join(str(x) + ' ' for x in lst[:10])} ... {''.join(str(x) + ' ' for x in lst[-10:])}] ({len(lst)} values)")
    if args.verify:
        print(f"Tape is {'NOT ' if lst != sorted(lst) else ''}sorted.")


if __name__ == "__main__":
    main()
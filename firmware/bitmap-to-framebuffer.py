import array
import argparse
import itertools
import skimage.io
import skimage.filters


def main():
    # Command line arguments
    parser = argparse.ArgumentParser(description = 'Convert a bitmap picture to raw data for a SSD1306 oled screen in horizontal addressing mode')
    parser.add_argument('--array-name', default = 'bitmap_data')
    parser.add_argument('input_path')

    args = parser.parse_args()

    # Load the input as gray scale
    img = skimage.io.imread(args.input_path, as_gray = True)
    
    # Filter the input to enforce a 1 bit image
    threshold = skimage.filters.threshold_otsu(img)
    img = img > threshold
    
    # Convertion to a byte array
    scanline_list = [array.array('B', [0] * img.shape[1]) for i in range(img.shape[0] // 8)]
    for i in range(img.shape[0]):
        for j in range(img.shape[1]):
            if img[i][j]:
                scanline_list[i // 8][j] |= 1 << (i % 8) 

    # Generate C code
    print('#include <stdint.h>')
    print('const __flash uint8_t') 
    print(f'{args.array_name}[{img.shape[0] * img.shape[1] // 8}] = {{')
    print(', '.join(f'0x{byte:02x}'for byte in itertools.chain(*scanline_list)))
    print('};')


if __name__ == "__main__":
    main()

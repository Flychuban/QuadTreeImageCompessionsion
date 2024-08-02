import imageio.v3 as iio
import os
import sys

def create_gif(image_folder, output_path):
    images = []
    for depth in range(0, int(sys.argv[3]) + 1):  # Assuming the max depth is passed as a third argument
        image_path = os.path.join(image_folder, f"result2_image_depth_{depth}.jpg")
        if os.path.exists(image_path):
            images.append(iio.imread(image_path))
        else:
            print(f"Warning: {image_path} does not exist")
    
    iio.imwrite(output_path, images, duration=2)

# Ensure the script is called with the correct arguments
if len(sys.argv) != 4:
    print("Usage: python create_gif.py <image_folder> <output_gif_path> <max_depth>")
else:
    image_folder = sys.argv[1]
    output_gif_path = sys.argv[2]
    create_gif(image_folder, output_gif_path)
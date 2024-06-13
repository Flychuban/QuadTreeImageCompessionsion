import imageio
import os

MAX_DEPTH = 10

def create_gif(image_folder, output_path):
    images = []
    for depth in range(MAX_DEPTH + 1):
        filename = os.path.join(image_folder, f'result_image_depth_{depth}.jpg')
        images.append(imageio.imread(filename))
    
    imageio.mimsave(output_path, images, duration=2)

# Specify the folder containing the images and the path to save the GIF
image_folder = "/Users/flychuban/Documents/compression_results/"
output_gif_path = "/Users/flychuban/Documents/compression_results/compression_stages.gif"

create_gif(image_folder, output_gif_path)
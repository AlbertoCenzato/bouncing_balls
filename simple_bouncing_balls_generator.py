import time
import multiprocessing
from bouncing_balls import generate_data, Config


if __name__ == '__main__':
    multiprocessing.freeze_support()
    config = Config(sequences=600, sequence_len=40, occlusion=False, balls=[1, 4], data_dir='C:\\Users\\micheluzzo\\Desktop\\data', 
                    balls_radius=3, screen_height=60, screen_width=60, mean_vel=6000)
    start = time.perf_counter()
    generate_data(config, suppress_output=False)
    print('Elapsed time: {}'.format(time.perf_counter() - start))
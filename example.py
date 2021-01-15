from bouncing_balls import BouncingBalls, Config, Kind

cfg = Config(
    sequences=6000, 
    sequence_len=40, # sequence length expressed in video frames
    occlusion=False, 
    balls=[3,6],  # the balls in in each sequence will be randomly choosen to be either 3 or 6
    data_dir="",
    balls_radius=5, 
    mean_vel=5000, 
    dof=2,
    screen_height=48, 
    screen_width=64, 
    channels_ordering=Kind.LAST, # or Kind.FIRST
    save_metadata=True  # unused parameter
    )

bb = BouncingBalls(cfg)
bb.generate(
    dataset_dir=".\data",  # output dir 
    suppress_output=False  # unused parameter
    )
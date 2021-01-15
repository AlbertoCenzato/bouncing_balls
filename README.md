# Install
Checkout repository
```
git clone https://github.com/AlbertoCenzato/bouncing_balls.git
cd bouncing_balls
git submodule init
git submodule update
```

Use pip to install
```
pip install <this-repo-checkout-dir>
```

# Usage
Basic usage
```python
from bouncing_balls import Config, BouncingBalls

cfg = Config(sequences=6000, sequence_len=40, balls=[1])
bb = BouncingBalls(cfg)
bb.generate("output-dir")
```

for a complete example with all available parametes see example.py
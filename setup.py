import setuptools

with open("README.md", "r") as fh:
    long_description = fh.read()

setuptools.setup(
    name="bouncing_balls",
    version="0.8.1",
    author="Alberto Cenzato",
    author_email="alberto.cenzato@outlook.it",
    description="Random bouncing balls dataset generator",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://bitbucket.org/AlbeCenz/bouncing_balls",
    packages=setuptools.find_packages(),
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: Microsoft :: Windows :: Windows 10",
    ],
)
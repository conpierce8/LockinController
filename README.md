# ABOUT

LockinController is an application for performing customizable parametric sweeps of
experiments controlled by Stanford Research Systems lock-in amplifiers, with
implementations in C++, MATLAB, and Python.


# INSTALLATION

## PYTHON

To set up the Python version of LockinController, refer to the following instructions.
The Windows instructions have been tested; Linux instructions have not (but hopefully
should work).


1.  Install [Miniconda](https://docs.conda.io/projects/miniconda/en/latest/#latest-miniconda-installer-links)

    If installing on Windows, we recommend using the "Just Me" installation option.
2.  __Linux:__ start a terminal window.

    __Windows:__ Start the Anaconda Prompt from the Start Menu.
3.  [Optional] Install [`mamba`](https://anaconda.org/conda-forge/mamba) by entering the
    following command in the terminal window:

    ```
    conda install -c conda-forge mamba
    ```

    This step is not necessary, but may speed up the installation process for you. If
    you wish to use `mamba`, replace "conda" with "mamba" in all of the commands that
    follow, e.g. replace

    ```
    conda create -n visa python
    ```

    with

    ```
    mamba create -n visa python
    ```
4. Install `conda-build`:

    ```
    conda install conda-build
    ```
5.  Create a new conda environment and install the packages required by
    LockinController:

    ```
    conda create -n visa -c conda-forge matplotlib numpy pyvisa 
    ```
6.  Activate the newly-created environment:

    ```
    conda activate visa
    ```
7.  Download the LockinController source code to your computer.
8.  Add the LockinController Python package as a development package to your conda
    environment, so that you can import it in Python scripts:

    ```
    conda develop "path/to/LockinController/python"
    ```

    Note that the LockinController source code contains a folder named `python`; this
    should be the last folder that you include in the path. For mamba users, note that
    as of version 1.5.3, mamba does not support the `develop` command, so you must use
    conda for this step.

To test your installation, start a Python interpreter by and enter the statement:

```
import lockin
```

If this command succeeds without errors, then you have successfully installed
LockinController.

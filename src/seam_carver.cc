#include "seam_carver.hpp"

// implement the rest of SeamCarver's functions here

int SeamCarver::FindMin(int prev_up, int prev_down, int prev_right) const {
  int min = prev_right;
  if (prev_up < min) {
    min = prev_up;
  }
  if (prev_down < min) {
    min = prev_down;
  }
  return min;
}

const ImagePPM& SeamCarver::GetImage() const { return image_; }

// returns the image's height
int SeamCarver::GetHeight() const { return height_; }

// returns the image's width
int SeamCarver::GetWidth() const { return width_; }

// Returns the energy of the pixel at (row, col) of the instance’s image.
// You may assume that row and col will always be within bounds.
int SeamCarver::GetEnergy(int row, int col) const {
  Pixel top = Pixel();
  Pixel bottom = Pixel();
  Pixel left = Pixel();
  Pixel right = Pixel();
  if (row == 0) {
    top = image_.GetPixel(height_ - 1, col);
    bottom = image_.GetPixel(row + 1, col);
  } else if (row == height_ - 1) {
    top = image_.GetPixel(row - 1, col);
    bottom = image_.GetPixel(0, col);
  } else {
    top = image_.GetPixel(row - 1, col);
    bottom = image_.GetPixel(row + 1, col);
  }
  int col_r =
      (top.GetRed() - bottom.GetRed()) * (top.GetRed() - bottom.GetRed());
  int col_g = (top.GetGreen() - bottom.GetGreen()) *
              (top.GetGreen() - bottom.GetGreen());
  int col_b =
      (top.GetBlue() - bottom.GetBlue()) * (top.GetBlue() - bottom.GetBlue());
  int adj_col = col_r + col_g + col_b;
  if (col == 0) {
    left = image_.GetPixel(row, width_ - 1);
    right = image_.GetPixel(row, col + 1);
  } else if (col == width_ - 1) {
    left = image_.GetPixel(row, col - 1);
    right = image_.GetPixel(row, 0);
  } else {
    left = image_.GetPixel(row, col - 1);
    right = image_.GetPixel(row, col + 1);
  }
  int row_r =
      (left.GetRed() - right.GetRed()) * (left.GetRed() - right.GetRed());
  int row_g = (left.GetGreen() - right.GetGreen()) *
              (left.GetGreen() - right.GetGreen());
  int row_b =
      (left.GetBlue() - right.GetBlue()) * (left.GetBlue() - right.GetBlue());
  int adj_row = row_r + row_g + row_b;
  return adj_col + adj_row;
}

int** SeamCarver::PopulateHenergies(int height_, int width_) const {
  int** henergies = new int*[height_];
  for (int row = 0; row < height_; ++row) {
    henergies[row] = new int[width_];
    for (int col = 0; col < width_; ++col) {
      henergies[row][col] = GetEnergy(row, col);
    }
  }
  for (int col = width_ - 2; col >= 0; --col) {
    for (int row = 0; row < height_; ++row) {
      int prev_up = 0;
      int prev_right = 0;
      int prev_down = 0;
      if (row == 0) {
        prev_up = kMaxInt;
        prev_down = henergies[row + 1][col + 1];
      } else if (row == height_ - 1) {
        prev_up = henergies[row - 1][col + 1];
        prev_down = kMaxInt;
      } else {
        prev_up = henergies[row - 1][col + 1];
        prev_down = henergies[row + 1][col + 1];
      }
      prev_right = henergies[row][col + 1];
      henergies[row][col] +=
          SeamCarver::FindMin(prev_up, prev_down, prev_right);
    }
  }
  return henergies;
}
// Returns the horizontal seam of image_ with the least amount of energy.
// Notice the array returned by this method is on the free store; keep in mind
// when the memory should be free’d.
int* SeamCarver::GetHorizontalSeam() const {
  int** henergies = PopulateHenergies(height_, width_);
  int first_col_min_index = 0;
  for (int row = 0; row < height_; row++) {
    if (henergies[row][0] < henergies[first_col_min_index][0]) {
      first_col_min_index = row;
    }
  }
  int* rowindex = new int[width_];
  int prev_row_index = first_col_min_index;
  rowindex[0] = prev_row_index;
  for (int col = 1; col < width_; ++col) {
    int adj_up_energy = 0;
    int adj_right_energy = 0;
    int adj_down_energy = 0;
    if (prev_row_index == 0) {
      adj_up_energy = kMaxInt;
      adj_down_energy = henergies[prev_row_index + 1][col];
    } else if (prev_row_index == height_ - 1) {
      adj_up_energy = henergies[prev_row_index - 1][col];
      adj_down_energy = kMaxInt;
    } else {
      adj_up_energy = henergies[prev_row_index - 1][col];
      adj_down_energy = henergies[prev_row_index + 1][col];
    }
    adj_right_energy = henergies[prev_row_index][col];
    PopulatingRowIndex(adj_up_energy,
                       adj_right_energy,
                       adj_down_energy,
                       rowindex,
                       col,
                       prev_row_index);
    prev_row_index = rowindex[col];
  }
  for (int i = 0; i < height_; ++i) {
    delete[] henergies[i];
  }
  delete[] henergies;
  henergies = nullptr;
  return rowindex;
}

void SeamCarver::PopulatingRowIndex(int adj_up_energy,
                                    int adj_right_energy,
                                    int adj_down_energy,
                                    int*& rowindex,
                                    int col,
                                    int prev_row_index) const {
  int min = adj_right_energy;
  rowindex[col] = prev_row_index;
  if (adj_up_energy < min) {
    rowindex[col] = prev_row_index - 1;
    min = adj_up_energy;
  }
  if (adj_down_energy < min) {
    rowindex[col] = prev_row_index + 1;
  }
}

int** SeamCarver::PopulateVenergies(int height_, int width_) const {
  int** venergies = new int*[height_];
  for (int row = 0; row < height_; ++row) {
    venergies[row] = new int[width_];
    for (int col = 0; col < width_; ++col) {
      venergies[row][col] = GetEnergy(row, col);
    }
  }
  for (int row = height_ - 2; row >= 0; --row) {
    for (int col = 0; col < width_; ++col) {
      int prev_left = 0;
      int prev_down = 0;
      int prev_right = 0;
      if (col == 0) {
        prev_left = kMaxInt;
        prev_right = venergies[row + 1][col + 1];
      } else if (col == width_ - 1) {
        prev_left = venergies[row + 1][col - 1];
        prev_right = kMaxInt;
      } else {
        prev_left = venergies[row + 1][col - 1];
        prev_right = venergies[row + 1][col + 1];
      }
      prev_down = venergies[row + 1][col];
      venergies[row][col] +=
          SeamCarver::FindMin(prev_right, prev_left, prev_down);
    }
  }
  return venergies;
}

int* SeamCarver::GetVerticalSeam() const {
  int** venergies = PopulateVenergies(height_, width_);
  int first_row_min_index = 0;
  for (int col = 0; col < width_; col++) {
    if (venergies[0][col] < venergies[0][first_row_min_index]) {
      first_row_min_index = col;
    }
  }
  int* colindex = new int[height_];
  int prev_col_index = first_row_min_index;
  colindex[0] = prev_col_index;
  for (int row = 1; row < height_; ++row) {
    int down_left_energy = 0;
    int down_down_energy = 0;
    int down_right_energy = 0;
    if (prev_col_index == 0) {
      down_left_energy = kMaxInt;
      down_right_energy = venergies[row][prev_col_index + 1];
    } else if (prev_col_index == width_ - 1) {
      down_left_energy = venergies[row][prev_col_index - 1];
      down_right_energy = kMaxInt;
    } else {
      down_left_energy = venergies[row][prev_col_index - 1];
      down_right_energy = venergies[row][prev_col_index + 1];
    }
    down_down_energy = venergies[row][prev_col_index];
    PopulatingColIndex(down_left_energy,
                       down_right_energy,
                       down_down_energy,
                       colindex,
                       row,
                       prev_col_index);
    prev_col_index = colindex[row];
  }
  for (int i = 0; i < height_; ++i) {
    delete[] venergies[i];
  }
  delete[] venergies;
  venergies = nullptr;
  return colindex;
}

void SeamCarver::PopulatingColIndex(int down_left_energy,
                                    int down_right_energy,
                                    int down_down_energy,
                                    int*& colindex,
                                    int row,
                                    int prev_col_index) const {
  int min = down_down_energy;
  colindex[row] = prev_col_index;
  if (down_left_energy < min) {
    colindex[row] = prev_col_index - 1;
    min = down_left_energy;
  }
  if (down_right_energy < min) {
    colindex[row] = prev_col_index + 1;
  }
}

void SeamCarver::RemoveHorizontalSeam() {
  // rowindex has seam indices
  int* rowindex = GetHorizontalSeam();

  image_.CarveHorizontalSeam(rowindex);
  height_ -= 1;
  delete[] rowindex;
}

void SeamCarver::RemoveVerticalSeam() {
  // colindex has seam indices
  int* colindex = GetVerticalSeam();

  image_.CarveVerticalSeam(colindex);

  width_ -= 1;
  delete[] colindex;
}

// given functions below, DO NOT MODIFY

SeamCarver::SeamCarver(const ImagePPM& image): image_(image) {
  height_ = image.GetHeight();
  width_ = image.GetWidth();
}

void SeamCarver::SetImage(const ImagePPM& image) {
  image_ = image;
  width_ = image.GetWidth();
  height_ = image.GetHeight();
}

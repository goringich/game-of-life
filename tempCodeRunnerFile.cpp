
    for (size_t i = 0; i < rows; i++) {
      for (size_t j = 0; j < cols; j++) {
        if (data[i * cols + j]->getSymbol() == u8"∞" && data[i * cols + j]->isMoved==false) { // If Prey
          bool preyMoved = false;
          while (!preyMoved) {
            int direction = rand() % 4; // 0: up, 1: down, 2: left, 3: right
            size_t ni = i;
            size_t nj = j;
            if (direction == 0) 
              ni--; // Move up
            else if (direction == 1) 
              ni++; // Move down
            else if (direction == 2)
              nj--; // Move left
            else
              nj++; // Move right
            if (ni >= 0 && ni < rows && nj >= 0 && nj < cols && data[ni * cols + nj]->getSymbol() == " ") {
                std::swap(data[i * cols + j], data[ni * cols + nj]);
                data[i * cols + j]->isMoved==true;
                preyMoved = true;
            }
          }

        }
      }
    }
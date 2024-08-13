import React, { StrictMode } from 'react';
import { createRoot } from 'react-dom/client';
import AppWrapper from './App';

const root = createRoot(document.getElementById('root'));

root.render(
  <StrictMode>
    <AppWrapper />
  </StrictMode>
);

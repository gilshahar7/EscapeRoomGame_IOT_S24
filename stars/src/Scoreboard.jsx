import React, { useEffect, useState } from 'react';
import { getDatabase, ref, onValue } from 'firebase/database';
import './Scoreboard.css';

const Scoreboard = () => {
  const [scores, setScores] = useState([]);

  useEffect(() => {
    const database = getDatabase();
    const scoresRef = ref(database, 'scores'); // Path to your scores in Firebase

    onValue(scoresRef, (snapshot) => {
      const data = snapshot.val();
      if (data) {
        const scoresArray = Object.keys(data).map((key) => ({
          name: key,
          score: data[key],
        }));

        // Parse "MM:SS" format and sort by remaining time
        scoresArray.sort((a, b) => {
          const [aMinutes, aSeconds] = a.score.split(':').map(Number);
          const [bMinutes, bSeconds] = b.score.split(':').map(Number);
          const aTotalSeconds = aMinutes * 60 + aSeconds;
          const bTotalSeconds = bMinutes * 60 + bSeconds;
          return aTotalSeconds - bTotalSeconds; // Sort in descending order
        });

        setScores(scoresArray);
      }
    });
  }, []);

  return (
    <div className="scoreboard-container">
      <h1 className="scoreboard-title">Scoreboard</h1>
      <table>
        <thead>
          <tr>
            <th>Rank</th>
            <th>Name</th>
            <th>Time</th>
          </tr>
        </thead>
        <tbody>
          {scores.map((score, index) => (
            <tr key={index} className={`rank-${index + 1}`}>
            <td className="scoreboard-rank"><strong>{index + 1}</strong></td>
            <td className="scoreboard-name"><strong>{score.name}</strong></td>
            <td className="scoreboard-score"><strong>{score.score}</strong></td>
          </tr>
          ))}
        </tbody>
      </table>
    </div>
  );
};

export default Scoreboard;
